#pragma once

#include "util.h"

#include <cstdint>

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

	bool fixed() {return fixed_;}

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
	 * Whether the page is fixed in memory.
	 * If this is false, the page will probably be flushed to disk soon.
	 */
	bool fixed_;

	friend BufferManager;
};
