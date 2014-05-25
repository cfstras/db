#pragma once

#include <memory>

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
	T lowestKey;
	PageID page;
};

/**
 * Describes a node page.
 */
template <class T>
struct BTreeNode {
	PageID upperPage;
	SlotID count;
	BTreeKP<T> children[0];
};

/**
 * Describes a key/value pair in a leaf.
 */
template <class T>
struct BTreeKV {
	T key;
	TID value;
};

/**
 * Describes a leaf.
 */
template <class T>
struct BTreeLeaf {
	PageID nextPage;
	SlotID count;
	BTreeKV<T> children[0];
};

/**
 * Describes a B-Tree page.
 * if isLeaf is true, _only_ the leaf field is to be used,
 * if isLeaf is false, _only_ the node field is to be used.
 */
template <class T>
struct BTreePage {
	bool isLeaf;
	union {
		BTreeNode<T> node;
		BTreeLeaf<T> leaf;
	};
};

#pragma pack(pop)

} // namespace

template <class T, class CMP>
class BTree {

	/**
	 * Creates a B-Tree object in this segment. Before first usage, it should
	 * be initialized with init().
	 */
	BTree(SegmentID segment, std::shared_ptr<BufferManager> bufferManager);

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

	std::shared_ptr<BufferManager> bufferManager_;

	SegmentID segment_;
	PageID headPageID;
	BufferFrame* headerFrame;
};
