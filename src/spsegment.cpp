#include "spsegment.h"

#include <cstring>
#include <cassert>

#include "buffermanager.h"

using namespace std;

SPSegment::SPSegment(SegmentID segment, shared_ptr<BufferManager> bufferManager)
	: SPSegment(segment, bufferManager, false)
{
}

SPSegment::SPSegment(SegmentID segment, shared_ptr<BufferManager> bufferManager, bool create) :
		bufferManager_(bufferManager),
		segment_(segment),
		headPageID(util::pageIDFromSegmentID(segment_)) {
	assert(segment >> 2 * 4 != 255); // this would be our slot marker
	headerFrame = &bufferManager_->fixPage(headPageID, true);
	header = (SegmentHeader*)headerFrame->getData();
	if (create) {
		memset(header, 0, sizeof(SegmentHeader));
	}
	// do some stuff?
	// check integrity?
}

SPSegment::~SPSegment() {
	bufferManager_->unfixPage(*headerFrame, true);
}

SlottedPage::SlottedPage(PageID pageID, shared_ptr<BufferManager> bm, bool exclusive) :
		bufferManager_(bm), dirty(false), pageID(pageID) {
	frame = &bufferManager_->fixPage(pageID, exclusive);
	header = reinterpret_cast<PageHeader*>(frame->getData());
	// TODO integrity check?
}

SlottedPage::~SlottedPage() {
	//TODO clean things up?
	bufferManager_->unfixPage(*frame, dirty);
}

void SlottedPage::init() {
	memset(header, 0, sizeof(PageHeader));
	header->dataStart = PAGE_SIZE;
	header->freeSpace = PAGE_SIZE - sizeof(PageHeader);
}

SlottedPage* SPSegment::getPageForTID(TID tid, bool exclusive) {
	PageID pageID = util::extractPageIDFromTID(tid);
	pageID = putSegmentInPageID(pageID);
	return new SlottedPage(pageID, bufferManager_, exclusive);
}

Slot* SPSegment::getSlotForTID(SlottedPage *page, TID tid) {
	uint16_t slotIndex = util::extractSlotIDFromTID(tid);
	Slot* slot = &page->header->slots[slotIndex];
	return slot;
}

void SPSegment::initializeSlot(Slot* slot) {
	slot->tid = 0; // clean out
	slot->__padding = 0xfade; // for initialization check
	slot->T = 255; // to indicate record is here
	slot->S = 0; // to indicate record was not moved from somewhere
	slot->offset = 0;
	slot->len = 0;
}

TID SPSegment::insert(const Record& r) {
	// find free page
	//TODO also look for fitting spaces
	//TODO also look for fragmented spaces
	//TODO implement FSI

	// check for maximum
	assert(r.len() <= PAGE_SIZE - sizeof(PageHeader) - sizeof(Slot));

	SlottedPage *page;
	bool foundOne = false;
	for (PageID pageID = 1; isPageInThisSegment(pageID) && !foundOne; pageID++) {
		PageID withSeg = putSegmentInPageID(pageID);
		page = new SlottedPage(withSeg, bufferManager_, true);
		// slot count * sizeof slot + sizeof PageHeader
		// should be smaller than (dataStart - record length)
		uint16_t freeSpaceAtStart = page->header->dataStart -
				page->header->count*sizeof(Slot) - sizeof(PageHeader);

		assert(freeSpaceAtStart <= page->header->freeSpace);

		if (freeSpaceAtStart >= r.len() + sizeof(Slot)) {
			foundOne = true;
		} else {
			//TODO page is full. compactify?
			delete page;
			page = nullptr;
		}
	}
	if (!foundOne) {
		PageID pageID = ++header->pageCount;
		PageID withSeg = putSegmentInPageID(pageID);
		page = new SlottedPage(withSeg, bufferManager_, true);
		page->init();
	}

	assert(page->header->freeSpace >= r.len() + sizeof(Slot));
	uint16_t slotIndex = page->header->firstFreeSlot++;
	page->header->count++;
	page->header->freeSpace -= r.len() + sizeof(Slot);

	Slot slot;
	initializeSlot(&slot);
	slot.offset = page->header->dataStart - r.len();
	slot.len = r.len();

	page->header->slots[slotIndex] = slot;
	memcpy(reinterpret_cast<char*>(page->header) + slot.offset,
			r.data(), slot.len);

	page->header->dataStart -= r.len();
	page->dirty = true;

	TID tid = slotIndex | (page->pageID << 4 * 4);
	delete page;
	return tid;
}

Record SPSegment::lookup(TID tid) {
	SlottedPage *page = getPageForTID(tid, false);
	Slot slot = *getSlotForTID(page, tid); // create copy of struct!
	SlotID slotIndex = util::extractSlotIDFromTID(tid);
	if (page->header->count == 0 || (slot.len == 0 && slot.offset == 0)
			|| page->header->count <= slotIndex) {
		// slot was deleted or did not exist
		delete page;
		return Record(0, nullptr);
	}
	assert(slot.__padding == 0xfade); // slot was not initialized

	if (slot.T != 255) {
		tid = slot.tid;
		delete page;
		return lookup(tid); //TODO add counter to prevent recursion
	}
	if (slot.S != 0) {
		// original TID is at page->header + offset
		// record is 8 bytes later
		slot.offset += 8;
	}
	Record r(slot.len, reinterpret_cast<char*>(page->header) + slot.offset);
	delete page;
	return r;
}

bool SPSegment::remove(TID tid) {
	SlottedPage *page = getPageForTID(tid, true);
	Slot *slot = getSlotForTID(page, tid);
	SlotID slotIndex = util::extractSlotIDFromTID(tid);
	assert(page->header->count >= 1); // cannot delete from empty page

	assert(page->header->count > slotIndex); // slot was probably removed
	assert(slot->__padding == 0xfade); // slot was not initialized

	if (slot->T != 255) {
		// slot->tid is other record
		// no data stored on this page
		// delete the referred one, too
		remove(slot->tid);
	} else {
		// ignore S, only go downwards the rabbit hole, not upwards
		if (page->header->firstFreeSlot-1 == slotIndex) {
			// only decrement the firstFreeSlot if this is the last one
			page->header->firstFreeSlot = slotIndex;
		}
		if (slot->S != 0) {
			// increase length for dataStart and freeSpace calculations
			slot->len += sizeof(TID);
		}
		if (page->header->dataStart == slot->offset) {
			page->header->dataStart += slot->len;
		}
		page->header->freeSpace += slot->len;
	}
	if (page->header->count-1 == slotIndex) {
		page->header->count = slotIndex;
	}
	initializeSlot(slot);
	page->dirty = true;
	delete page;
	return true;
}

//TODO implement update()
