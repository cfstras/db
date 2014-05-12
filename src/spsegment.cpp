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

SlottedPage::SlottedPage(PageID pageID, shared_ptr<BufferManager> bm) :
		bufferManager_(bm), dirty(false), pageID(pageID) {
	frame = &bufferManager_->fixPage(pageID, true);
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

SlottedPage* SPSegment::getPageForTID(TID tid) {
	PageID pageID = util::extractPageIDFromTID(tid);
	pageID = putSegmentInPageID(pageID);
	return new SlottedPage(pageID, bufferManager_);
}

Slot* SPSegment::getSlotForTID(SlottedPage *page, TID tid) {
	uint16_t slotIndex = util::extractSlotIDFromTID(tid);
	assert(page->header->count > slotIndex);

	Slot* slot = &page->header->slots[slotIndex];
	assert(slot->__padding == 0xfade); // slot was not initialized
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

	// check for maximum
	assert(r.len() <= PAGE_SIZE-sizeof(PageHeader));

	SlottedPage *page;
	bool foundOne = false;
	for (PageID pageID = 1; isPageInThisSegment(pageID) && !foundOne; pageID++) {
		PageID withSeg = putSegmentInPageID(pageID);
		page = new SlottedPage(withSeg, bufferManager_);
		// slot count (-1 b/c PageHeader has 1) * sizeof slot + sizeof PageHeader
		// should be smaller than dataStart - new record length
		// cast that one to signed for overflows
		if ((page->header->count-1)*sizeof(Slot) + sizeof(PageHeader) <
			static_cast<int64_t>(page->header->dataStart) - r.len()) {
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
		page = new SlottedPage(withSeg, bufferManager_);
		page->init();
	}

	uint16_t slotIndex = page->header->firstFreeSlot++;
	page->header->count++;
	page->header->freeSpace -= r.len();

	Slot slot;
	initializeSlot(&slot);
	slot.offset = page->header->dataStart - r.len();
	slot.len = r.len();

	page->header->slots[slotIndex] = slot;
	memcpy(reinterpret_cast<char*>(page->header) + slot.offset,
			r.data(), slot.len);

	page->header->dataStart -= slot.len;
	page->dirty = true;

	TID tid = slotIndex | (page->pageID << 4 * 4);
	delete page;
	return tid;
}

Record SPSegment::lookup(TID tid) {
	SlottedPage *page = getPageForTID(tid);
	Slot slot = *getSlotForTID(page, tid); // create copy of struct!

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
	SlottedPage *page = getPageForTID(tid);
	Slot *slot = getSlotForTID(page, tid);
	if (slot->T != 255) {
		// slot->tid is other record
		// delete that one, too
		remove(slot->tid);
	} else {
		// ignore S, only go downwards, not upwards
		page->header->firstFreeSlot = util::extractSlotIDFromTID(tid);
		page->header->freeSpace += slot->len;
	}
	initializeSlot(slot);
	page->dirty = true;
	delete page;
	return true;
}

//TODO implement update()
