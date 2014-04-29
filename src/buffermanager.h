#pragma once

#include <unordered_map>
#include <vector>
#include <mutex>
#include <cstdint>

#include "util.h"
#include "bufferframe.h"

class BufferManager {
public:
	/**
	 * Creates a new instance which keeps up to size frames in memory at one time.
	 */
	BufferManager(unsigned size);

	/**
	 * Writes all dirty frames and frees all resources.
	 */
	~BufferManager();

	/**
	 * Retrieves a frame by its pageId. The corresponding frame is kept in
	 * memory until unfixPage() is called with it.
	 * Can fail if no free frame is available and no other can be freed.
	 */
	BufferFrame& fixPage(uint64_t pageId, bool exclusive);

	/**
	 * Returns control of a page to the BufferManager.
	 * If isDirty is set, the page will be written to disk (possibly
	 * asynchronously).
	 * Any access to the BufferFrame after calling this method will produce
	 * undefined behaviour.
	 */
	void unfixPage(BufferFrame& frame, bool isDirty);

	unsigned size() {return size_;}

private:
	DISALLOW_COPY_AND_ASSIGN(BufferManager);

	void flushNow(BufferFrame& frame);
	void queueFlush(BufferFrame& frame);

	void load(BufferFrame& frame, uint64_t pageId);

	uint64_t offset(uint64_t pageId);

	/**
	 * Frees a loaded, unfixed page
	 */
	void freePage();

	unsigned size_;

	/**
	 * Map from pageId to BufferFrame pointers.
	 */
	std::unordered_map<uint64_t, BufferFrame*> slots;
	std::mutex slots_mutex;
};
