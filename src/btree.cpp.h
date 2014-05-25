#include "btree.h"

using namespace std;

template <typename T, typename CMP>
BTree<T, CMP>::BTree(SegmentID segment, shared_ptr<BufferManager> bufferManager) :
		bufferManager_(bufferManager),
		segment_(segment)
{
	headPageID = util::pageIDFromSegmentID(segment_);

}

template <typename T, typename CMP>
BTree<T, CMP>::~BTree() {
	//TODO

}
