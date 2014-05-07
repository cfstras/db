#pragma once

#include <memory>

#include "buffermanager.h"
#include "record.h"
#include "util.h"

namespace {

// 16-bit in-page slot addresses --> 65K max page size

typedef struct {
	uint64_t pageCount;
	//TODO add freeSpaceInventory
} SegmentHeader;

#pragma pack(push) //TODO is this correct?
#pragma pack(1)

typedef struct {
	union {
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
		};
	};
} Slot;

#pragma pack(pop)

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
	Slot slots[1];
} PageHeader;

class SlottedPage {
public:
	SlottedPage(BufferFrame *frame, PageID pageID);
	SlottedPage(SlottedPage& a) = delete;

	BufferFrame* frame;
	PageHeader* header;

	// the page ID, with segment id inside
	PageID pageID;
};

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

private:
	DISALLOW_COPY_AND_ASSIGN(SPSegment);

	/** helper methods **/

	// builds the page ID for a TID
	inline PageID pageIDFromTID(TID tid) {
		return putSegmentInPageID(util::extractPageIDFromTID(tid));
	}

	inline PageID putSegmentInPageID(PageID pageID) {
		//                                       12 nibbles
		return (static_cast<PageID>(segment_) << 12 * 4) | pageID;
	}

	/**
	 * Load a page from this segment. Upper 16 bits of pageID must be 0.
	 * If page is out of range, null is returned.
	 */
	SlottedPage* loadPage(PageID pageID);

	/**
	 * Unloads a page.
	 * @param dirty specifies whether the page could be dirty.
	 */
	void unloadPage(SlottedPage* page, bool dirty);

	/**
	 * Create a new page at the end of this segment
	 */
	SlottedPage* createPage();

	/** fields **/

	std::shared_ptr<BufferManager> bufferManager_;

	SegmentID segment_;
	PageID headPageID;
	BufferFrame* headerFrame;
	SegmentHeader* header;

};
