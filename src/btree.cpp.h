#include "btree.h"

#include <cstring>

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
	headPage->count = 0;

	unloadPage(headFrame, true);
}

template <typename T, typename CMP>
tuple<PageID, vector<BufferFrame*>, bool> BTree<T, CMP>::lookupPage(T key, bool keepLocks) {
	vector<BufferFrame*> frames;
	CMP cmp;

	PageID pageID = 0; //TODO insert head pageID here
	BufferFrame *frame = loadPage(pageID, keepLocks);
	frames.push_back(frame);
	BTreePage<T> *page = (BTreePage<T>*) frame->getData();
	bool leafFull;
	while (true) {
		if (page->isLeaf) {
			leafFull = (sizeof(BTreePage<T>) +
					sizeof(BTreeKV<T>) * (page->count+1))
					> PAGE_SIZE;
			break;
		}

		BTreeNode<T> *node = &page->node;
		PageID candidate = node->upperPage;

		for (uint16_t i = 0; i < page->count; i++) {
			BTreeKP<T> *child = &node->children[i];
			if (cmp(key, child->lowestKey)) {
				// key < pageStart
				break;
			} else {
				// key >= pageStart
				candidate = child->page;
			}
		}
		BufferFrame *lastFrame = frame;
		if (candidate == 0) { //TODO insert head pageID here
			//TODO
		}
		// load the next page
		pageID = candidate;

		frame = loadPage(candidate, keepLocks);
		if (!keepLocks) { // lock-coupling
			unloadPage(lastFrame, false);
		}
		frames.push_back(frame);
		page = (BTreePage<T>*) frame->getData();
	}

	if (!keepLocks) {
		unloadPage(frame, false);
		frames.clear();
	}
	return make_tuple(pageID, frames, leafFull);
}

template <typename T, typename CMP>
TID BTree<T, CMP>::insert(T key, TID tid) {
	auto ret = lookupPage(key, false);
	PageID pageID = get<0>(ret);
	bool leafFull = get<2>(ret);
	CMP cmp;

	if (leafFull) {
		assert(false); //TODO implement
	}
	BufferFrame *frame = loadPage(pageID, true);
	BTreePage<T> *page = (BTreePage<T>*) frame->getData();
	assert(page->isLeaf); // lookupPage should always return a leaf.
	BTreeLeaf<T> *leaf = &page->leaf;
	TID oldTID = 0;

	uint16_t count = page->count;
	uint16_t index = count;
	for (uint16_t i = 0; i < count; i++) {
		BTreeKV<T> *child = &leaf->children[i];
		if (cmp(key, child->key)) {
			// key < slot
			continue;
		}
		// it should go here, but we have to move some stuff first
		index = i;
		break;
	}
	if (index < count) {
		if (leaf->children[index].key == key) {
			oldTID = leaf->children[index].value;
		}
		// move!
		memmove(&leaf->children[index+1], &leaf->children[index],
				sizeof(BTreeKV<T>) * (count - index));
	}
	page->count++;
	leaf->children[index].value = tid;
	leaf->children[index].key = key;

	unloadPage(frame, true);
	return oldTID;
}

template <typename T, typename CMP>
TID BTree<T, CMP>::lookup(T key) {
	//TODO implement
	return 0;
}
