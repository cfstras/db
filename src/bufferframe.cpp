#include "bufferframe.h"

BufferFrame::BufferFrame() :
	pageId_(0),
	fixed_(false)
{
	data = malloc(PAGE_SIZE);
}

BufferFrame::~BufferFrame() {
	free data;
}
