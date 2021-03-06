#include "buffermanager.h"

#include <iostream>
#include <map>
#include <unistd.h>
#include <fcntl.h>

#include "bufferframe.h"

using namespace std;

BufferManager::BufferManager(unsigned size, shared_ptr<FileManager> fm) :
	fileManager_(fm),
	size_(size),
	slots(size)
{
	// TODO check for files
	srand(time(0));
}

BufferManager::BufferManager(unsigned size) :
	BufferManager(size, shared_ptr<FileManager>(FileManager::instance()))
{
}

BufferManager::~BufferManager() {
	slot_condition.notify_all();
	lock_guard<mutex> g(slots_mutex);
	for(const auto &entry : slots) {
		BufferFrame* f = entry.second;
		if (f != nullptr) {
			if (f->dirty()) {
				flushNow(*f);
			}
			delete f;
		}
	}
}


BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
	BufferFrame *frame;

	slots_mutex.lock();
	auto size = slots.size();
	auto it = slots.find(pageId);
	slots_mutex.unlock();

	if (it == slots.end()) {
		// make a new one
		if (size >= size_) {
			freePage();
		}
		frame = new BufferFrame();
		load(*frame, pageId);
	} else {
		frame = it->second;
	}

	// get the locks
	if (exclusive) {
		util::checkReturn("locking frame exclusive",
			pthread_rwlock_wrlock(&frame->latch_));
	} else {
		util::checkReturn("locking frame",
			pthread_rwlock_rdlock(&frame->latch_));
	}

	{	lock_guard<mutex> g(slots_mutex);
		slots[pageId] = frame;
	}
	return *frame;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
	frame.dirty_ |= isDirty;
	// release our lock

	util::checkReturn("unlocking frame",
		pthread_rwlock_unlock(&frame.latch_));
	slot_condition.notify_one();
}

uint64_t BufferManager::offset(uint64_t pageId) {
	return PAGE_SIZE * (pageId & 0x0000ffffffffffffULL);
}

void BufferManager::load(BufferFrame& frame, uint64_t pageId) {
	util::checkReturn("locking frame for load",
		pthread_rwlock_wrlock(&frame.latch_));

	frame.pageId_ = pageId;
	// first two bytes are the chunk id
	int fd = fileManager_->getFile(pageId);
	auto bytes = pread(fd, frame.getData(), PAGE_SIZE, offset(pageId));
	if (bytes == -1) {
		util::checkReturn("loading page "+to_string(pageId), errno);
	} else if (bytes != PAGE_SIZE) {
		// this should only happen when the file is shorter than requested.
		// as the file will get the correct size once we flush, just do nothing here.
	}
	util::checkReturn("unlocking frame after load",
		pthread_rwlock_unlock(&frame.latch_));
}

void BufferManager::flushNow(BufferFrame& frame) {
	// we only need a read lock for flushing
	util::checkReturn("locking frame for flush",
		pthread_rwlock_rdlock(&frame.latch_));

	auto pageId = frame.pageId();
	int fd = fileManager_->getFile(pageId);

	auto ret = posix_fallocate(fd, offset(pageId), PAGE_SIZE);
	util::checkReturn("allocating space for page "+to_string(pageId), ret);

	auto bytes = pwrite(fd, frame.getData(), PAGE_SIZE, offset(pageId));
	if (bytes == -1) {
		util::checkReturn("saving page "+to_string(pageId), errno);
	} else if (bytes != PAGE_SIZE) {
		// oh no!
		util::checkReturn("flushing page did not write all data", -1);
		//TODO prettier exceptions
	}
	frame.dirty_ = false;

	util::checkReturn("unlocking frame",
		pthread_rwlock_unlock(&frame.latch_));
}

void BufferManager::queueFlush(BufferFrame& frame) {
	//TODO concurrency
	flushNow(frame);
}

void BufferManager::freePage() { // free page! what did he do wrong?
	unique_lock<mutex> lock(slots_mutex);
	slot_condition.wait(lock, [&lock, this] {
		return tryFreeingPage(lock);
	});
}

bool BufferManager::tryFreeingPage(unique_lock<mutex> &lock) {
	BufferFrame *frame;

	auto it = slots.begin();
	// start at random element
	advance(it, rand() % slots.size());
	lock.unlock();

	// iterate through them, find an unfixed one
	size_t i = 0;
	int err;
	while ((err = pthread_rwlock_trywrlock(&it->second->latch_)) == EBUSY) {
		if (err != 0 && err != EBUSY) {
			util::checkReturn("getting frame lock", err);
		}
		lock.lock();

		++it; ++i;
		if (i > slots.size()) break;
		if (it == slots.end()) it = slots.begin();

		lock.unlock();
	}
	if (it == slots.end()) {
		return false;
	}

	// clear it from the map
	lock.lock();
	frame = it->second;
	slots.erase(it);
	lock.unlock(); // release lock while flushing

	util::checkReturn("unlocking frame for free",
		pthread_rwlock_unlock(&frame->latch_));

	if (frame->dirty()) {
		flushNow(*frame);
	}
	delete frame;

	lock.lock();
	return true;
}
