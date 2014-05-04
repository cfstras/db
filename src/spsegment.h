#pragma once

#include "record.h"
#include "util.h"

class SPSegment {
public:
	/**
	 * Loads a slotted page segment from a pageID.
	 * The page at that address must contain a valid SPSegment header.
	 */
	SPSegment(uint64_t headPageId);

	/**
	 * Creates a new SPSegment starting at a given page id.
	 * Overwrites any data which is currently stored there.
	 * @param slotSize slot size in bytes
	 */
	SPSegment(uint64_t start, uint64_t slotSize);

	/**
	 * Inserts a record into this slotted page.
	 * @return the assigned TID
	 */
	TID insert(const Record& record);

	/**
	 * Removes a record from this page and updates page header accordingly.
	 * @return true on success, false if TID does not correspond to a Record
	 */
	bool remove(TID tid);

	/**
	 * Returns a copy of the record associated with TID tid.
	 * To update changed data, use update().
	 */
	Record lookup(TID tid);

	/**
	 * Updates the record at TID with the content of the given record.
	 * @return true on success, false false if TID does not correspond to a Record
	 */
	bool update(TID tid, const Record& record);

private:
	DISALLOW_COPY_AND_ASSIGN(SPSegment);

	//TODO

};
