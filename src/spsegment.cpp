#include "spsegment.h"

#include <iostream>
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

BufferFrame* SPSegment::loadPage(PageID pageID, bool exclusive) {
	BufferFrame *frame = &bufferManager_->fixPage(pageID, exclusive);
	return frame;
	// TODO integrity check?
}

void SPSegment::unloadPage(BufferFrame *frame, bool dirty) {
	//TODO clean things up?
	bufferManager_->unfixPage(*frame, dirty);
}

void SPSegment::initPage(PageHeader *header) {
	memset(header, 0, sizeof(PageHeader));
	header->dataStart = PAGE_SIZE;
	header->freeSpace = PAGE_SIZE - sizeof(PageHeader);
}

BufferFrame* SPSegment::getPageForTID(TID tid, bool exclusive) {
	PageID pageID = util::extractPageIDFromTID(tid);
	pageID = putSegmentInPageID(pageID);
	return loadPage(pageID, exclusive);
}

Slot* SPSegment::getSlotForTID(PageHeader* page, TID tid) {
	uint16_t slotIndex = util::extractSlotIDFromTID(tid);
	Slot* slot = &page->slots[slotIndex];
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

	BufferFrame *frame;
	PageHeader *page;
	bool foundOne = false;
	PageID pageID, withSeg;
	uint16_t freeSpaceAtStart;
	for (pageID = 1; isPageInThisSegment(pageID); pageID++) {
		withSeg = putSegmentInPageID(pageID);
		frame = loadPage(withSeg, true);
		page = (PageHeader*)frame->getData();
		// slot count * sizeof slot + sizeof PageHeader
		// should be smaller than (dataStart - record length)
		freeSpaceAtStart = page->dataStart -
				page->count*sizeof(Slot) - sizeof(PageHeader);

		assert(freeSpaceAtStart <= page->freeSpace);

		if (freeSpaceAtStart >= r.len() + sizeof(Slot)) {
			foundOne = true;
			break;
		} else {
			//TODO page is full. compactify?
			unloadPage(frame, false);
			page = nullptr; frame = nullptr;
		}
	}
	if (!foundOne) {
		pageID = ++header->pageCount;
		withSeg = putSegmentInPageID(pageID);
#ifndef SILENT
		cerr << "ins: create p " << hex << withSeg << dec << " for l="
				<< (r.len() + sizeof(Slot)) << endl;
#endif
		frame = loadPage(withSeg, true);
		page = (PageHeader*)frame->getData();
		initPage(page);
	} else {
#ifndef SILENT
		cerr << "ins: use    p " << hex << withSeg << dec << " for l="
				<< (r.len() + sizeof(Slot)) << ", has " << freeSpaceAtStart
				<< " left" << endl;
#endif
	}

	assert(page->freeSpace >= r.len() + sizeof(Slot));
	uint16_t slotIndex = page->firstFreeSlot++;
	page->count++;
	page->freeSpace -= r.len() + sizeof(Slot);

	Slot slot;
	initializeSlot(&slot);
	slot.offset = page->dataStart - r.len();
	slot.len = r.len();

	page->slots[slotIndex] = slot;
	memcpy(reinterpret_cast<char*>(page) + slot.offset,
			r.data(), slot.len);

	page->dataStart -= r.len();

	TID tid = slotIndex | (pageID << 4 * 4);
	unloadPage(frame, true);
	return tid;
}

Record SPSegment::lookup(TID tid) {
	BufferFrame *frame = getPageForTID(tid, false);
	PageHeader *header = (PageHeader*)frame->getData();
	Slot slot = *getSlotForTID(header, tid); // create copy of struct!
	SlotID slotIndex = util::extractSlotIDFromTID(tid);
	if (header->count == 0 || (slot.len == 0 && slot.offset == 0)
			|| header->count <= slotIndex) {
		// slot was deleted or did not exist
		unloadPage(frame, false);
		return Record(0, nullptr);
	}
	assert(slot.__padding == 0xfade); // slot was not initialized

	if (slot.T != 255) {
		tid = slot.tid;
		unloadPage(frame, false);
		return lookup(tid); //TODO add counter to prevent recursion
	}
	if (slot.S != 0) {
		// original TID is at header + offset
		// record is 8 bytes later
		slot.offset += 8;
	}
	Record r(slot.len, reinterpret_cast<char*>(header) + slot.offset);
	unloadPage(frame, false);
	return r;
}

bool SPSegment::remove(TID tid) {
	BufferFrame *frame = getPageForTID(tid, true);
	PageHeader *header = (PageHeader*)frame->getData();
	Slot *slot = getSlotForTID(header, tid);
	SlotID slotIndex = util::extractSlotIDFromTID(tid);
	TID newTID = 0;

	assert(header->count >= 1); // cannot delete from empty page
	assert(header->count > slotIndex); // slot was probably removed
	assert(slot->__padding == 0xfade); // slot was not initialized

	if (slot->T != 255) {
		// slot->tid is other record
		// no data stored on this page
		// delete the referred one, too
		newTID = slot->tid;
	} else {
		// ignore S, only go downwards the rabbit hole, not upwards
		if (header->firstFreeSlot-1 == slotIndex) {
			// only decrement the firstFreeSlot if this is the last one
			header->firstFreeSlot = slotIndex;
		}
		if (slot->S != 0) {
			// increase length for dataStart and freeSpace calculations
			slot->len += sizeof(TID);
		}
		if (header->dataStart == slot->offset) {
			header->dataStart += slot->len;
		}
		header->freeSpace += slot->len;
	}
	if (header->count-1 == slotIndex) {
		header->count = slotIndex;
	}
	initializeSlot(slot);
	unloadPage(frame, true);

	if (newTID != 0) {
		remove(newTID);
	}
	return true;
}

//TODO implement update()
