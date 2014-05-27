#include <gtest/gtest.h>

#include <cstdlib>
#include "btree.h"

using namespace std;

namespace btree_test {

// Comparator
class UInt64Comp {
public:
	bool operator()(uint64_t a, uint64_t b) const {
		return a<b;
	}
};

// fixed-size string
template <unsigned len>
struct Char {
	char data[len];
};

class BTreeTest : public ::testing::Test {
protected:
	void SetUp() {
		srand(0);
		fm = shared_ptr<FileManager>(new FileManager("test_data"));
		bm = shared_ptr<BufferManager>(new BufferManager(128, fm.get()));
	}

	shared_ptr<FileManager> fm;
	shared_ptr<BufferManager> bm;
};

TEST_F(BTreeTest, Construct) {
	BTree<uint64_t, UInt64Comp> tree(4, bm);
}

TEST_F(BTreeTest, Init) {
	BTree<uint64_t, UInt64Comp> tree(4, bm);
	tree.init();
}


TEST_F(BTreeTest, Insert) {
	BTree<uint64_t, UInt64Comp> tree(4, bm);
	tree.init();

	uint64_t key = rand();
	TID value = rand();
	TID old = tree.insert(key, value);
	EXPECT_EQ(0, old) << "Tree should be empty but returned "<< old;
}


TEST_F(BTreeTest, InsertLookup) {
	BTree<uint64_t, UInt64Comp> tree(4, bm);
	tree.init();

	uint64_t key = rand();
	TID value = rand();
	TID old = tree.insert(key, value);
	EXPECT_EQ(0, old) << "Tree should be empty but returned "<< old <<
			" (inserted "<<value<<", key="<<key<<")";

	TID v2 = tree.lookup(key);
	EXPECT_EQ(value, v2) << "Tree returned wrong value " << v2 <<
			" (inserted "<<value<<", key="<<key<<")";
}

TEST_F(BTreeTest, InsertTwice) {
	BTree<uint64_t, UInt64Comp> tree(4, bm);
	tree.init();

	uint64_t key = rand();
	TID value = rand();
	TID old = tree.insert(key, value);
	EXPECT_EQ(0, old) << "Tree should be empty but returned "<< old <<
			" (inserted "<<value<<", key="<<key<<")";

	TID v2 = rand();
	old = tree.insert(key, v2);
	EXPECT_EQ(value, old) << "Second insert returned "<< old <<
			" (inserted "<<v2<<", key="<<key<<", after inserting "<<value<<")";

	TID v3 = tree.lookup(key);
	EXPECT_EQ(v2, v3) << "Tree returned wrong value " << v3 <<
			" (inserted "<<v2<<", key="<<key<<")";
}

} // namespace
