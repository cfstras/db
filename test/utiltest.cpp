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

} // namespace
