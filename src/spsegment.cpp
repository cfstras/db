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

SlottedPage* SPSegment::loadPage(PageID pageID) {
	assert(0 == pageID >> 12 * 4);

	// page id 0 is the header
	// page id 1 is available when count >= 1
	if (pageID > header->pageCount) {
		return nullptr;
	}
	pageID = putSegmentInPageID(pageID);
	BufferFrame &frame = bufferManager_->fixPage(pageID, true);

	SlottedPage* page = new SlottedPage(&frame, pageID);
	// TODO integrity check?
	return page;
}

SlottedPage* SPSegment::createPage() {
	PageID pageID = ++header->pageCount;
	pageID = putSegmentInPageID(pageID);
	BufferFrame &frame = bufferManager_->fixPage(pageID, true);
	SlottedPage* page = new SlottedPage(&frame, pageID);

	memset(page->header, 0, sizeof(PageHeader));
	page->header->dataStart = PAGE_SIZE;
	page->header->freeSpace = PAGE_SIZE - sizeof(PageHeader);

	return page;
}

void SPSegment::unloadPage(SlottedPage* page, bool dirty) {
	//TODO clean things up?
	bufferManager_->unfixPage(*page->frame, dirty);
	delete page;
}

SlottedPage::SlottedPage(BufferFrame *f, PageID page) :
	frame(f),
	header((PageHeader*)f->getData()),
	pageID(page) {
}

TID SPSegment::insert(const Record& r) {
	// find free page

	bool foundOne = false;
	SlottedPage *page;
	for (PageID pageID = 1; (page = loadPage(pageID)) != nullptr && !foundOne; pageID++) {
		if (page->header->count*sizeof(Slot) + sizeof(PageHeader) <
			page->header->dataStart - r.len()) {
			foundOne = true;
		} else {
			//TODO page is full. compactify?
			// unload page
			unloadPage(page, false);
			page = nullptr;
		}
	}
	if (page == nullptr) {
		page = createPage();
	}

	uint16_t slotIndex = page->header->firstFreeSlot++;
	page->header->count++;
	page->header->freeSpace -= r.len();

	Slot slot;
	slot.tid = 0;
	slot.T = 255; slot.S = 0;
	slot.offset = page->header->dataStart;
	slot.len = r.len();

	page->header->slots[slotIndex] = slot;
	memcpy(page->header + slot.offset, r.data(), slot.len);

	page->header->dataStart -= slot.len;

	TID tid = slotIndex | (page->pageID << 4 * 4);
	return tid;
}

Record SPSegment::lookup(TID t) {
	//TODO implement
	string str("nope");
	return Record(str.length()+1, str.c_str());
}
