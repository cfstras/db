#include "spsegment.h"

#include <cstring>

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
	header = (PageHeader*)headerFrame->getData();
	if (create) {
		memset(header, 0, sizeof(PageHeader));
	}
	// do some stuff?
	// check integrity?
}

SPSegment::~SPSegment() {
	bufferManager_->unfixPage(*headerFrame, true);
}
