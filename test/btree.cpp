#include <gtest/gtest.h>

#include "btree.h"

using namespace std;

namespace btree_test {

// Comparator
class UInt64Comp {
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


} // namespace
