#pragma once

#include <unordered_map>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <cstdint>

#include "util.h"
#include "filemanager.h"
#include "bufferframe.h"

class BufferManager {
public:
	/**
	 * Creates a new instance which keeps up to size frames in memory at one time.
	 */
	BufferManager(unsigned size);

	/**
	 * Creates a new instance which keeps up to size frames in memory at one time.
	 * @param fm to use a different-than-default FileManager.
	 */
	BufferManager(unsigned size, std::shared_ptr<FileManager> fm);

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

	static uint64_t offset(uint64_t pageId);

	/**
	 * Frees a loaded, unfixed page.
	 * If none is available, waits until there is one.
	 * Afterwards, slots.size() is guaranteed to be 1 smaller.
	 */
	void freePage();

	/**
	 * Tries to free a page, returns the success.
	 */
	bool tryFreeingPage(std::unique_lock<std::mutex> &lock);

	std::shared_ptr<FileManager> fileManager_;
	unsigned size_;

	/**
	 * Map from pageId to BufferFrame pointers.
	 */
	std::unordered_map<uint64_t, BufferFrame*> slots;
	std::mutex slots_mutex;

	/**
	 * Used to wait for free pages.
	 * Notified whenever a page gets thrown out.
	 */
	std::condition_variable slot_condition;
};
