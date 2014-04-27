#include <cstdlib>

#include "bufferframe.h"

using namespace std;

BufferFrame::BufferFrame() :
	pageId_(0),
	fixed_(false),
	exclusive_(false),
	dirty_(false)
{
	data_ = malloc(PAGE_SIZE);
}

BufferFrame::~BufferFrame() {
	if (dirty_) {
		//TODO this should never happen!!!
	}
	free(data_);
}
