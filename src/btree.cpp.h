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

template <typename T, typename CMP>
pair<PageID, vector<BufferFrame*>> BTree<T, CMP>::lookupPage(T key, bool keepLocks) {
	vector<BufferFrame*> frames;

	PageID lastPage = 0;
	BufferFrame *frame = loadPage(lastPage, keepLocks);
	frames.push_back(frame);
	BTreePage<T> *page = (BTreePage<T>*) frame->getData();

	bool done = false;
	while (!done) {
		if (!page->isLeaf) {
			PageID candidate = page->upperPage;
			BTreeNode<T> *node = &page->node;
			for (uint16_t i = 0; i < node->count; i++) {
				BTreeKP<T> *child = node->children[i];
				if (CMP(key, child->lowestKey)) {
					// key < pageStart
					break;
				} else {
					// key >= pageStart
					candidate = child->page;
				}
			}
			// load the next page
			BufferFrame *lastFrame = frame;
			lastPage = candidate;

			frame = loadPage(candidate, keepLocks);
			if (!keepLocks) { // lock-coupling
				unloadPage(lastFrame, false);
			}

			frames.push_back(frame);
			page = (BTreePage<T>*) frame->getData();

		} else {
			BTreeLeaf<T> *leaf = &page->leaf;
			//TODO
		}
	}
	//TODO implement
	return pair<PageID, vector<BufferFrame*>>(0, frames);
	//TODO release any locks
}

template <typename T, typename CMP>
TID BTree<T, CMP>::insert(T key, TID tid) {
	//TODO implement
	return 0;
}

template <typename T, typename CMP>
TID BTree<T, CMP>::lookup(T key) {
	//TODO implement
	return 0;
}
