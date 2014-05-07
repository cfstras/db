#include <gtest/gtest.h>

#include "util.h"

namespace {

TEST(UtilTest, extractSegmentFromPageID) {
	EXPECT_EQ(0,		util::extractSegmentFromPageID(1));
	EXPECT_EQ(0,		util::extractSegmentFromPageID(0x0000100000000000ULL));
	EXPECT_EQ(1,		util::extractSegmentFromPageID(0x0001000000000000ULL));
	EXPECT_EQ(0x00ff,	util::extractSegmentFromPageID(0x00ff000000000000ULL));
	EXPECT_EQ(0xff00,	util::extractSegmentFromPageID(0xff00000000000000ULL));
	EXPECT_EQ(0xcafe,	util::extractSegmentFromPageID(0xcafe000000000000ULL));
}

TEST(UtilTest, extractPageFromPageID) {
	EXPECT_EQ(1,						util::extractPageFromPageID(1));
	EXPECT_EQ(0,						util::extractPageFromPageID(0xcafe000000000000ULL));
	EXPECT_EQ(0x0000f00000000000ULL,	util::extractPageFromPageID(0x0000f00000000000ULL));
	EXPECT_EQ(0x0000000000000001ULL,	util::extractPageFromPageID(0x000f000000000001ULL));
	EXPECT_EQ(0x0000f00000000000ULL,	util::extractPageFromPageID(0x000ff00000000000ULL));
}

TEST(UtilTest, extractSlotIDFromTID) {
	EXPECT_EQ(1,		util::extractSlotIDFromTID(1));
	EXPECT_EQ(0,		util::extractSlotIDFromTID(0xcafecafecafe0000ULL));
	EXPECT_EQ(0xf000,	util::extractSlotIDFromTID(0xcafecafecafef000ULL));
	EXPECT_EQ(0xbeef,	util::extractSlotIDFromTID(0x000000000000beefULL));
	EXPECT_EQ(0xbeef,	util::extractSlotIDFromTID(0xcafecafecafebeefULL));
}

TEST(UtilTest, extractPageIDFromTID) {
	EXPECT_EQ(0,						util::extractPageIDFromTID(0x000000000000cafeULL));
	EXPECT_EQ(0x0000cafecafecafeULL,	util::extractPageIDFromTID(0xcafecafecafe0000ULL));
	EXPECT_EQ(0x0000cafecafecafeULL,	util::extractPageIDFromTID(0xcafecafecafebeefULL));
}

} // namespace
