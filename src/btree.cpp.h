#include "btree.h"

#include <cstring>

using namespace std;

template <typename T, typename CMP>
BTree<T, CMP>::BTree(SegmentID segment, shared_ptr<BufferManager> bufferManager) :
		bufferManager_(bufferManager),
		segment_(segment),
		rootPageID(0), // TODO materialize from metadata segment
		numPages(0) // TODO same
{

}

template <typename T, typename CMP>
BTree<T, CMP>::~BTree() {
	//TODO save to metasegment
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
	BufferFrame *frame = loadPage(rootPageID, true);
	BTreePage<T> *page = (BTreePage<T>*) frame->getData();

	page->isLeaf = false;
	BTreeNode<T> *headNode = &page->node;
	PageID leafPage = rootPageID+1;
	headNode->upperPage = leafPage; // also add leaf
	page->count = 0;
	unloadPage(frame, true);

	frame = loadPage(leafPage, true);
	page = (BTreePage<T>*) frame->getData();
	page->isLeaf = true;
	page->count = 0;
	page->leaf.nextPage = 0;
	unloadPage(frame, true);

	numPages = 2; //TODO save
}

template <typename T, typename CMP>
tuple<PageID, vector<BufferFrame*>, bool> BTree<T, CMP>::lookupPage(T key, bool keepLocks) {
	vector<BufferFrame*> frames;
	CMP cmp;

	PageID pageID = rootPageID;
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
		if (candidate == rootPageID || candidate == -1) {
			pageID = -1;
			break;
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

	assert(pageID != -1);
	if (leafFull) {
		// split the leaf
		assert(false); //TODO implement
	}
	BufferFrame *frame = loadPage(pageID, true);
	BTreePage<T> *page = (BTreePage<T>*) frame->getData();
	assert(page->isLeaf); // should always return a leaf

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
		if (eq(cmp, leaf->children[index].key, key)) {
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
	auto ret = lookupPage(key, false);
	PageID pageID = get<0>(ret);
	CMP cmp;

	if (pageID == -1) { // key not found at all
		return 0;
	}
	BufferFrame *frame = loadPage(pageID, true);
	BTreePage<T> *page = (BTreePage<T>*) frame->getData();
	if (!page->isLeaf) { // page is not a leaf ==> key does not exist
		unloadPage(frame, false);
		return 0;
	}

	BTreeLeaf<T> *leaf = &page->leaf;
	uint16_t count = page->count;
	for (uint16_t i = 0; i < count; i++) {
		BTreeKV<T> *child = &leaf->children[i];
		if (cmp(key, child->key)) {
			// key < slot
			continue;
		}
		if (!eq(cmp, key, child->key)) {
			unloadPage(frame, false);
			return 0;
		}
		unloadPage(frame, false);
		return child->value;
	}
	unloadPage(frame, false);
	return 0;
}
