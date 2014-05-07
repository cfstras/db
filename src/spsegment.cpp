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

unique_ptr<SlottedPage> SPSegment::loadPage(PageID pageID) {
	assert(0 == pageID >> 12 * 4);

	// page id 0 is the header
	// page id 1 is available when count >= 1
	if (pageID > header->pageCount) {
		return nullptr;
	}
	pageID = putSegmentInPageID(pageID);
	BufferFrame &frame = bufferManager_->fixPage(pageID, true);

	unique_ptr<SlottedPage> page(new SlottedPage(&frame));
	// TODO integrity check?
	return page;
}

SlottedPage::SlottedPage(BufferFrame *f) :
	frame(f), header((PageHeader*)f->getData()) {
}

TID SPSegment::insert(const Record& r) {
	//TODO implement
	return 0;
}

Record SPSegment::lookup(TID t) {
	//TODO implement
	string str("nope");
	return Record(str.length()+1, str.c_str());
}
