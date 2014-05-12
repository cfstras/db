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

TID SPSegment::insert(const Record& r) {
	// find free page
	SlottedPage *page;
	bool foundOne = false;
	for (PageID pageID = 1; isPageInThisSegment(pageID) && !foundOne; pageID++) {
		PageID withSeg = putSegmentInPageID(pageID);
		page = new SlottedPage(withSeg, bufferManager_);
		if (page->header->count*sizeof(Slot) + sizeof(PageHeader) <
			page->header->dataStart - r.len()) {
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
	slot.tid = 0;
	slot.T = 255; // to indicate record is here
	slot.S = 0; // to indicate record was not moved from somewhere
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

Record SPSegment::lookup(TID t) {
	uint16_t slotIndex = util::extractSlotIDFromTID(t);
	PageID pageID = util::extractPageIDFromTID(t);
	pageID = putSegmentInPageID(pageID);
	SlottedPage *page = new SlottedPage(pageID, bufferManager_);
	assert(page->header->count > slotIndex);
	Slot slot = page->header->slots[slotIndex];

	if (slot.T != 255) {
		t = slot.tid;
		delete page;
		return lookup(t);
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
