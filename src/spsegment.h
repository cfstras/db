#pragma once

#include <memory>
#include <cassert>

#include "buffermanager.h"
#include "record.h"
#include "util.h"

namespace {

// 16-bit in-page slot addresses --> 64KiB max page size

// Structure of the Slotted Page Segment Header
struct SegmentHeader {
	uint64_t pageCount;
	//TODO add freeSpaceInventory
};

// * 2 because 4 bits per FSI entry --> 2 entries per byte
//uint64_t numPagesPerFSI = (PAGE_SIZE - sizeof(SegmentHeader)) * 2;
// one entry is one nibble
//uint64_t numEntriesInFSI = numPagesPerFSI / 2;

// every (numPagesPerFSI+1)th page is an FSI page.

struct FSI : SegmentHeader {
	uint8_t spaces[];
};

#pragma pack(push)
#pragma pack(1)

// Structure of a Page Slot
typedef union {
	uint64_t tid;

	struct {
		uint8_t T; // if != 255, TID points to other record

		/**
		 * if 0, item is at offset, length
		 * else item was moved from somewhere:
		 * item is at offset+(8bytes), len,
		 * offset contains original TID
		 */
		uint8_t S;
		uint16_t __padding;

		uint16_t offset;
		uint16_t len;
		// if offset == len == 0, slot is free
	};
} Slot;

#pragma pack(pop)

// Structure of a Slotted Page Header
typedef struct {
	// total number of slots
	uint16_t count;
	// first free slot id
	uint16_t firstFreeSlot;
	// lower end of the data
	uint16_t dataStart;
	// space that would be available after compacting
	uint16_t freeSpace;
	// pointer to the slots. length is count.
	Slot slots[0];
} PageHeader;

} // namespace

class SPSegment {
public:
	/**
	 * Loads a slotted page segment from a segment id.
	 * page 0 at that address must contain a valid SPSegment header.
	 */
	SPSegment(SegmentID segmentID, std::shared_ptr<BufferManager> bufferManager);

	/**
	 * Creates a new SPSegment in that id, if create is true
	 */
	SPSegment(SegmentID segmentID, std::shared_ptr<BufferManager> bufferManager,
		bool create);

	~SPSegment();

	/**
	 * Inserts a record into this slotted page segment.
	 * @return the assigned TID
	 */
	TID insert(const Record& record);

	/**
	 * Removes a record from this segment and updates page header accordingly.
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

	SegmentID segment() { return segment_; }

	/** helper methods **/

	inline bool isPageInThisSegment(PageID pageID) {
		// page id 0 is the header
		// page id 1 is available when count >= 1
		return util::extractPageFromPageID(pageID) <= header->pageCount;
	}

	inline PageID putSegmentInPageID(PageID pageID) {
		assert(pageID >> 12*4 == 0); // pageID should not have segment in it
		//                                       12 nibbles
		return (static_cast<PageID>(segment_) << 12 * 4) | pageID;
	}

	// builds the page ID for a TID
	inline PageID pageIDFromTID(TID tid) {
		return putSegmentInPageID(util::extractPageIDFromTID(tid));
	}

private:
	DISALLOW_COPY_AND_ASSIGN(SPSegment);

	BufferFrame* loadPage(PageID pageID, bool exclusive);
	void unloadPage(BufferFrame *frame, bool dirty);
	void initPage(PageHeader *header);

	void compactifyPage(PageHeader *page);
	void moveBackCount(PageHeader *page);

	/**
	 * Loads and returns the page at this TID
	 */
	BufferFrame* getPageForTID(TID tid, bool exclusive);

	/**
	 * Gets the slot for this TID given the loaded page
	 */
	Slot* getSlotForTID(PageHeader *page, TID tid);

	void initializeSlot(Slot* slot);

	/** fields **/

	std::shared_ptr<BufferManager> bufferManager_;

	SegmentID segment_;
	PageID headPageID;
	BufferFrame* headerFrame;
	SegmentHeader* header; //TODO race-condition here!
};
