#include <gtest/gtest.h>

#include "buffermanager.h"

using namespace std;

namespace {

TEST(BufferManagerTest, InitAndDestruct) {
	BufferManager b(1);
	EXPECT_EQ(1, b.size());
}

TEST(BufferManagerTest, CanFix) {
	BufferManager b(1);
	BufferFrame &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());
	b.unfixPage(f, false);

	BufferFrame &f2 = b.fixPage(2, false);
	EXPECT_EQ(2, f2.pageId());
	b.unfixPage(f2, true);
}

} // namespace
