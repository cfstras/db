#pragma once

#include "util.h"

#include <mutex>
#include <cstdint>
#include <pthread.h>

class BufferManager;

class BufferFrame {
public:
	/**
	 * Returns a pointer to the actual data
	 */
	void* getData() {return data_;}

	/**
	 * Returns the page id
	 */
	uint64_t pageId() {return pageId_;}

	bool dirty() {return dirty_;}

private:
	DISALLOW_COPY_AND_ASSIGN(BufferFrame);

	BufferFrame();
	~BufferFrame();

	// Pointer to the data.
	void* data_;

	/**
	 * Holds the page id of this frame.
	 * The first 16 bits determine the location on disk.
	 */
	uint64_t pageId_;

	/**
	 * Read&Write latches. To read, acquire read lock.
	 * To write, acquire both locks (at the same time!).
	 */
	pthread_rwlock_t latch_;

	/**
	 * Whether this frame has dirty data in it.
	 * If true, this frame needs to be flushed to disk when it gets deleted.
	 */
	bool dirty_;

	// I feel dirty.
	friend BufferManager;
};
