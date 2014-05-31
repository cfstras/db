#pragma once

#include <memory>
#include <vector>
#include <tuple>
#include <mutex>
#include <atomic>

#include "util.h"
#include "buffermanager.h"

namespace {

#pragma pack(push)
#pragma pack(1)

template <typename T>
struct BTreeKV {
	union {
		PageID page; // in a node
		TID value; // in a leaf
	};
	T key;
};

/**
 * Describes a B-Tree page.
 */
template <typename T>
struct BTreePage {
	bool isLeaf;
	uint16_t count;
	//TODO test if padding here improves performance
	union {
		PageID upperPage; // in a node
		PageID nextPage; // in a leaf
	};
	BTreeKV<T> children[0];
};

#pragma pack(pop)

} // namespace

template <typename T, typename CMP>
class BTree {
public:
	/**
	 * Creates a B-Tree object in this segment. Before first usage, it should
	 * be initialized with init().
	 */
	BTree(SegmentID segment, std::shared_ptr<BufferManager> bufferManager);

	/**
	 * Unloads a B-Tree.
	 */
	~BTree();

	/**
	 * Initialize this B-Tree. All data in this segment will be lost.
	 */
	void init();

	/**
	 * Inserts a (key,TID) tuple into this tree.
	 * @return the TID previously associated with this key, or 0.
	 */
	TID insert(T key, TID tid);

	/**
	 * Erase a key from this tree.
	 * @return the TID associated with this key before erasing.
	 */
	TID erase(T key);

	/**
	 * Search for a key in this tree.
	 * @return the TID associated with the given key or 0.
	 */
	TID lookup(T key);

	static inline bool eq(CMP cmp, T a, T b) {
		return !(cmp(a, b)) && !(cmp(b, a));
	}

private:
	DISALLOW_COPY_AND_ASSIGN(BTree);

	inline PageID putSegmentInPageID(PageID pageID) {
		assert(pageID >> 12*4 == 0); // pageID should not have segment in it
		//                                       12 nibbles
		return (static_cast<PageID>(segment_) << 12 * 4) | pageID;
	}

	BufferFrame *loadPage(PageID pageID, bool exclusive);

	void unloadPage(BufferFrame *frame, bool dirty);

	/**
	 * Looks up the page where this key would go.
	 * @param keepLocks whether to hold the locks. If true, the corresponding
	 *        BufferFrame objects are passed along and valid. If false,
	 *        lock-coupling is used and any returned BufferFrames are invalid.
	 * @return a tuple of the would-be PageID, a vector of BufferFrames and
	 *         a boolean whether the leaf is full.
	 */
	std::tuple<PageID, std::vector<BufferFrame*>, bool> lookupPage(
			T key, bool keepLocks);

	void *initLeaf(BTreePage<T> *page);

	/**
	 * Splits the leaf, splitting parents as necessary.
	 * @return tuple of (new leaf PageID, new leaf BufferFrame)
	 */
	std::tuple<PageID, BufferFrame*> splitUpwards(T key, PageID oldLeafPageID);

	/** fields **/

	std::shared_ptr<BufferManager> bufferManager_;

	const SegmentID segment_;

	std::atomic<PageID> rootPageID;
	std::atomic<PageID> numPages;

};

// hax hax
#include "btree.cpp.h"
