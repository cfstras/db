#include <cstdlib>

#include "bufferframe.h"

using namespace std;

BufferFrame::BufferFrame() :
	pageId_(0),
	dirty_(false)
{
	util::checkReturn("creating frame latch",
		pthread_rwlock_init(&latch_, nullptr));
	data_ = malloc(PAGE_SIZE);
}

BufferFrame::~BufferFrame() {
	int err = 0;
	while ((err = pthread_rwlock_destroy(&latch_)) == EBUSY) {
		// get a write lock
		pthread_rwlock_wrlock(&latch_);
		// unlock to destroy
		pthread_rwlock_unlock(&latch_);
	}
	util::checkReturn("freeing frame", err);

	if (dirty_) {
		//TODO this should never happen!!!
	}
	free(data_);
}
