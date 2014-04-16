#include "buffermanager.h"

#include <map>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include "bufferframe.h"
#include "filemanager.h"

using namespace std;

BufferManager::BufferManager(unsigned size) :
	size_(size),
	slots(size)
{
	// check for files
	srand(time(0));
}

BufferManager::~BufferManager() {
	for(const auto &entry : slots) {
		BufferFrame* f = entry.second;
		if (f != nullptr) {
			if (f->fixed()) {
				throw exception(); //TODO oh no!
			}
			if (f->dirty()) {
				flushNow(*f);
			}
			delete f;
		}
	}
}


BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
	//TODO thread-safety
	//TODO currently, all is exclusive
	auto it = slots.find(pageId);
	BufferFrame *frame;

	if (it == slots.end()) {
		// make a new one
		if (slots.size() >= size_) {
			freePage();
		}
		frame = new BufferFrame();
		load(*frame, pageId);
	} else {
		frame = it->second;
	}

	// check if it is fixed
	if (frame->fixed()) {
		// this frame is already in use!
		//TODO wait for it to be freed
		throw exception();
	}

	frame->fixed_ = true;
	slots[pageId] = frame;
	return *frame;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
	frame.dirty_ |= isDirty;
	//TODO if it was exclusive, set that to false
	frame.fixed_ = false;
	queueFlush(frame);
}

void BufferManager::load(BufferFrame& frame, uint64_t pageId) {
	frame.pageId_ = pageId;
	// first two bytes are the chunk id
	size_t offset = pageId & 0x0000ffffffffffff;
	int fd = FileManager::instance()->getFile(pageId);
	auto bytes = pread(fd, frame.getData(), PAGE_SIZE, offset);
	if (bytes == -1) {
		util::checkReturn("loading page "+to_string(pageId)+" from file "+
			to_string(pageId), errno);
	} else if (bytes != PAGE_SIZE) {
		// this should only happen when the file is shorter than requested.
		// as the file will get the correct size once we flush, just do nothing here.
	}
}

void BufferManager::flushNow(BufferFrame& frame) {
	//TODO
}

void BufferManager::queueFlush(BufferFrame& frame) {
	//TODO concurrency
	flushNow(frame);
}

void BufferManager::freePage() { // free page! what did he do wrong?
	// start at random element
	auto it = slots.begin();
	advance(it, rand() % slots.size());

	size_t i = 0;
	while (it->second->fixed()) {
		++it;
		++i;
		if (i > slots.size()) {
			break;
		}
		if (it == slots.end()) it = slots.begin();
	}
	if (it == slots.end() || it->second == nullptr || it->second->fixed()) {
		throw exception(); //TODO more pretty exceptions
		//TODO wait for a free frame
	}

	BufferFrame* frame = it->second;
	slots.erase(it);

	flushNow(*frame);
	delete frame;
}

