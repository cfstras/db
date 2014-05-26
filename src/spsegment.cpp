#include "spsegment.h"

#include <iostream>
#include <algorithm>
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

void SPSegment::compactifyPage(PageHeader *page) {
	/*
	 * First, we load all slot pointers into a vector. Then, we sort the slots
	 * by their offset in descending order.
	 * Then, we can safely move them one after another and
	 * finally set the new data in the header.
	 */

	vector<pair<Slot*, SlotID>> slots;
	slots.reserve(page->count);
	for (SlotID slotIndex = 0; slotIndex < page->count; slotIndex++) {
		slots.emplace_back(pair<Slot*, SlotID>(&page->slots[slotIndex], slotIndex));
	}
	sort(slots.begin(), slots.end(),
		[](const pair<Slot*, SlotID> &a, const pair<Slot*, SlotID> &b){
			if (a.first->T != 255) {
				if (b.first->T != 255) {
					return false;
				} else {
					return true;
				}
			} else if (b.first->T != 255) {
				return false;
			}
			if (a.first->len == 0) {
				if (b.first->len == 0) {
					return true;
				} else {
					return false;
				}
			} else if (b.first->len == 0) {
				return true;
			}
			return a.first->offset > b.first->offset;
		});

	SlotID dataStart = PAGE_SIZE;
	SlotID firstFreeSlot = page->count;
	for (pair<Slot*, SlotID> &entry : slots) {
		Slot* slot = entry.first;
		if (slot->T != 255) continue;
		if (slot->offset == 0 && slot->len == 0) {
			if (entry.second < firstFreeSlot) firstFreeSlot = entry.second;
			continue;
		}
		Slot oldSlot = *entry.first;

		slot->offset = dataStart - slot->len;
		assert(slot->offset >= oldSlot.offset); // slot moved backwards
		if (slot->S != 0) oldSlot.len += sizeof(TID);
		if (slot->offset != oldSlot.offset) {
			memmove(reinterpret_cast<char*>(page) + slot->offset,
					reinterpret_cast<char*>(page) + oldSlot.offset,
					oldSlot.len);
		}

		dataStart -= oldSlot.len;
	}
	page->dataStart = dataStart;
	page->firstFreeSlot = firstFreeSlot;
	moveBackCount(page);
	// recalc freeSpace
	page->freeSpace = dataStart - page->count * sizeof(Slot) - sizeof(PageHeader);
}

void SPSegment::moveBackCount(PageHeader *page) {
	for (SlotID slotIndex = page->count-1;;slotIndex--) {
		const Slot slot = page->slots[slotIndex];
		if (slot.offset == 0 && slot.len == 0) {
			page->count = slotIndex;
			page->freeSpace += sizeof(Slot);
		} else {
			break;
		}
		if (slotIndex == 0) break;
	}
}

TID SPSegment::insert(const Record& r) {
	// find free page
	//TODO use empty slots, but always beginning of data
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
		} else if (page->freeSpace >= r.len() + sizeof(Slot)) {
			compactifyPage(page);

#ifdef DEBUG
			freeSpaceAtStart = page->dataStart -
				page->count*sizeof(Slot) - sizeof(PageHeader);
			assert(freeSpaceAtStart <= page->freeSpace);
#endif

			foundOne = true;
			break;
		} else {
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
	// search next firstFreeSlot
	if (slotIndex == page->count) {
		page->count++;
	} else {
		while (page->firstFreeSlot < page->count) {
			//TODO remove needs firstFreeSlot fixed
			const Slot slot = page->slots[page->firstFreeSlot];
			if (slot.T == 255 && slot.len == 0 && slot.offset == 0) {
				break;
			} else {
				page->firstFreeSlot++;
			}
		}
		if (page->firstFreeSlot == page->count) {
			page->count++;
		}
	}

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
		if (slot->S != 0) {
			// increase length for dataStart and freeSpace calculations
			slot->len += sizeof(TID);
		}
		if (header->dataStart == slot->offset) { //TODO also do these for free slots before this one
			header->dataStart += slot->len;
		}
		header->freeSpace += slot->len;
	}
	initializeSlot(slot);
	moveBackCount(header);
#ifdef DEBUG
	freeSpaceAtStart = page->dataStart -
			page->count*sizeof(Slot) - sizeof(PageHeader);
	assert(freeSpaceAtStart <= page->freeSpace);
#endif
	unloadPage(frame, true);

	if (newTID != 0) {
		remove(newTID);
	}
	return true;
}

//TODO implement update()
