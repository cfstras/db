#pragma once

#include <memory>
#include <vector>
#include <tuple>

#include "util.h"
#include "buffermanager.h"

namespace {

#pragma pack(push)
#pragma pack(1)

/**
 * Describes a key/child page pair in a node.
 */
template <class T>
struct BTreeKP {
	PageID page;
	T lowestKey;
};

/**
 * Describes a node page.
 */
template <class T>
struct BTreeNode {
	PageID upperPage;
	uint64_t size;
	//TODO test if padding here improves performance
	BTreeKP<T> children[0];
};

/**
 * Describes a key/value pair in a leaf.
 */
template <class T>
struct BTreeKV {
	TID value;
	T key;
};

/**
 * Describes a leaf.
 */
template <class T>
struct BTreeLeaf {
	PageID nextPage;
	//TODO test if padding here improves performance
	BTreeKV<T> children[0];
};

/**
 * Describes a B-Tree page.
 * If isLeaf is true, _only_ the leaf field is to be used,
 * If isLeaf is false, _only_ the node field is to be used.
 */
template <class T>
struct BTreePage {
	bool isLeaf;
	uint16_t count;
	//TODO test if padding here improves performance
	union {
		BTreeNode<T> node;
		BTreeLeaf<T> leaf;
	};
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

	/** fields **/

	std::shared_ptr<BufferManager> bufferManager_;

	SegmentID segment_;
	PageID headPageID;
};

// hax hax
#include "btree.cpp.h"
