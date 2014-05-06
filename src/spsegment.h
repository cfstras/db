#pragma once

#include "record.h"
#include "util.h"

namespace {

// 16-bit in-page slot addresses --> 65K max page size

#pragma pack(push) //TODO is this correct?
#pragma pack(1)

typedef struct {
	union {
		uint64_t TID;

		struct {
			uint8_t T; // if != 255, TID points to other record
			uint8_t S; // if 0, item is at offset
			uint16_t offset;
			uint16_t len;

			uint16_t __padding;
		};
	};
} Slot;

#pragma pack(pop)

typedef struct {
	// total number of slots
	uint16_t count;
	// offset to the last-inserted date. (topmost used byte)
	uint16_t lastData;
	// pointer to the slots. length is count.
	Slot slots[1];
} PageHeader;

} // namespace

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
