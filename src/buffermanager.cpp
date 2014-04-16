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
		if (entry.second != nullptr) {
			delete entry.second;
		}
	}
}


BufferFrame& BufferManager::fixPage(uint64_t pageId, bool exclusive) {
	//TODO thread-safety
	auto it = slots.find(pageId);
	BufferFrame *frame;

	if (it == slots.end()) {
		// make a new one
		if (slots.size() >= size_) {
			freePage();
		}
		frame = new BufferFrame();
	} else {
		frame = it->second;
	}

	// check if it is fixed
	if (frame->fixed()) {
		// this frame is already in use!
		//TODO what to do?
		throw exception();
	}

	if (frame->pageId() != pageId) {
		// flush this page NOW
		flushNow(*frame);

		// load the correct one
		load(*frame, pageId);
	}

	frame->fixed_ = true;
	slots[pageId] = frame;
	return *frame;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
	frame.dirty_ |= isDirty;
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
	// get random element
	auto it = slots.begin();
	advance(it, rand() % slots.size());
	BufferFrame* frame = it->second;
	slots.erase(it);

	flushNow(*frame);
	delete frame;
}

