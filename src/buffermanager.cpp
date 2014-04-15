#include "buffermanager.h"

#include <map>
#include <cstdlib>
#include <ctime>

#include "bufferframe.h"

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
	//TODO
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

	slots[pageId] = frame;
	return *frame;
}

void BufferManager::unfixPage(BufferFrame& frame, bool isDirty) {
	frame.dirty_ |= isDirty;
	queueFlush(frame);
}

void BufferManager::load(BufferFrame& frame, uint64_t pageId) {
	frame.pageId_ = pageId;
	//TODO
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

