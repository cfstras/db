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
	PageID leafPage = rootPageID+1;
	page->upperPage = leafPage; // also add leaf
	page->count = 0;
	unloadPage(frame, true);

	frame = loadPage(leafPage, true);
	page = (BTreePage<T>*) frame->getData();
	initLeaf(page);

	numPages = 2; //TODO save
}

template <typename T, typename CMP>
void *BTree<T, CMP>::initLeaf(BTreePage<T> *page) {
	page->isLeaf = true;
	page->count = 0;
	page->nextPage = 0;
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

		PageID candidate = page->upperPage;

		for (uint16_t i = 0; i < page->count; i++) {
			BTreeKV<T> *child = &page->children[i];
			if (cmp(key, child->key)) {
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
tuple<PageID, BufferFrame*> BTree<T, CMP>::splitUpwards(T key, PageID oldLeafPage) {
	CMP cmp;
	PageID oldLeafPageGet;
	vector<BufferFrame*> frames;
	bool full;
	tie(oldLeafPageGet, frames, full) = lookupPage(key, true);

	assert(full);
	assert(oldLeafPageGet == oldLeafPage);
	assert(frames.back()->pageID() == oldLeafPage);

	size_t index = frames.size()-1;
	PageID oldPageID, newPageID;
	BufferFrame *oldFrame, newFrame;
	BTreePage<T> *oldPage, newPage;

	T overflowKey; TID overflowValue; boolean overflow = false;
	while (full) {
		// split the leaf
		oldFrame = frames[index];
		oldPageID = oldFrame->pageID();
		BTreePage<T> *oldPage = (BTreePage<T>*)oldFrame->getData();

		PageID newPageID = numPages++;
		BufferFrame *newFrame = loadPage(newPageID, true);
		BTreePage<T> *newPage = (BTreePage<T>*) newFrame->getData();
		initLeaf(newPage);

		oldPage->nextPage = newPageID;
		T newKey = oldPage->children[oldPage->count/2].key;
		T oldKey = oldPage->children[0].key;

		for (uint16_t i = oldPage->count/2, i2 = 0; i < oldPage->count;
					i++, i2++) {
			newPage->children[i2] = oldPage->children[i];
		}
		oldPage->count = oldPage->count/2;
		newPage->count = (oldPage->count+1)/2;
		if (overflow) {
			overflow = false;
			newPage->children[newPage->count].value = overflowValue;
			newPage->children[newPage->count].key = overflowKey;
			newPage->count++;
		}

		unloadPage(oldFrame, true);
		unloadPage(newFrame, true);

		if (index == 0) {
			//TODO adjust root pageID
			rootPageID = newPageID;
			break;
		}
		//TODO fixup parent page if not root
		oldFrame = frames[index];
		oldPage = (BTreePage*)oldFrame->getData();
		uint16_t inRootInd = 0;
		while (true) {
			assert(inRootInd < oldPage->count);
			if (eq(cmp,oldPage->children[inRootInd].key, oldKey) {
				break;
			}
			inRootInd++;
		}
		// move right here

		if(index == 0) break;
		index--;
	}
	//TODO return
}

template <typename T, typename CMP>
TID BTree<T, CMP>::insert(T key, TID tid) {
	auto ret = lookupPage(key, false);
	PageID pageID = get<0>(ret);
	bool leafFull = get<2>(ret);
	CMP cmp;

	assert(pageID != -1);

	if (leafFull) {
		tie(pageID, frame) = splitUpwards(pageID);
	} else {
		BufferFrame *frame = loadPage(pageID, true);
	}
	BTreePage<T> *page = (BTreePage<T>*) frame->getData();
	assert(page->isLeaf); // should always return a leaf

	TID oldTID = 0;
	uint16_t count = page->count;
	uint16_t index = count;
	for (uint16_t i = 0; i < count; i++) {
		BTreeKV<T> *child = &page->children[i];
		if (cmp(key, child->key)) {
			// key < slot
			continue;
		}
		// it should go here, but we have to move some stuff first
		index = i;
		break;
	}
	if (index < count) {
		if (eq(cmp, page->children[index].key, key)) {
			oldTID = page->children[index].value;
		}
		// move!
		memmove(&page->children[index+1], &page->children[index],
				sizeof(BTreeKV<T>) * (count - index));
	}
	page->count++;
	page->children[index].value = tid;
	page->children[index].key = key;

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

	uint16_t count = page->count;
	for (uint16_t i = 0; i < count; i++) {
		BTreeKV<T> *child = &page->children[i];
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
