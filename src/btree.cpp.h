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

template <typename T, typename CMP>
BufferFrame* BTree<T, CMP>::loadPage(PageID pageID, bool exclusive) {
	PageID withSeg = putSegmentInPageID(pageID);
	BufferFrame *frame = &bufferManager_->fixPage(withSeg, exclusive);
	return frame;
}

template <typename T, typename CMP>
void BTree<T, CMP>::unloadPage(BufferFrame *frame, bool dirty) {
	bufferManager_->unfixPage(*frame, dirty);
}

template <typename T, typename CMP>
void BTree<T, CMP>::init() {
	BufferFrame *headFrame = loadPage(0, true);

	BTreePage<T> *headPage = (BTreePage<T>*) headFrame->getData();
	headPage->isLeaf = false;
	BTreeNode<T> *headNode = &headPage->node;
	headNode->upperPage = 0;
	headNode->count = 0;

	unloadPage(headFrame, true);
}
