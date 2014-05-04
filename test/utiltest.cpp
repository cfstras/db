#include <gtest/gtest.h>

#include "util.h"

namespace {

TEST(UtilTest, chunkId) {
	EXPECT_EQ(0, util::chunkId(1));
	EXPECT_EQ(0, util::chunkId(0x0000100000000000LL));
	EXPECT_EQ(1, util::chunkId(0x0001000000000000LL));
	EXPECT_EQ(0x00ff, util::chunkId(0x00ff000000000000LL));
	EXPECT_EQ(0xff00, util::chunkId(0xff00000000000000LL));
	EXPECT_EQ(0xcafe, util::chunkId(0xcafe000000000000LL));
}

TEST(UtilTest, extractPage) {
	EXPECT_EQ(1, util::extractPage(1));
	EXPECT_EQ(0, util::extractPage(0xcafe000000000000LL));
	EXPECT_EQ(0x0000f00000000000LL, util::extractPage(0x0000f00000000000LL));
	EXPECT_EQ(0x0000000000000001LL, util::extractPage(0x000f000000000001LL));
	EXPECT_EQ(0x0000f00000000000LL, util::extractPage(0x000ff00000000000LL));
}

} // namespace
