#include <gtest/gtest.h>

#include "util.h"

using namespace util;

namespace {

TEST(UtilTest, extractSegmentFromPageID) {
	EXPECT_EQ(0,		extractSegmentFromPageID(1));
	EXPECT_EQ(0,		extractSegmentFromPageID(0x0000100000000000ULL));
	EXPECT_EQ(1,		extractSegmentFromPageID(0x0001000000000000ULL));
	EXPECT_EQ(0x00ff,	extractSegmentFromPageID(0x00ff000000000000ULL));
	EXPECT_EQ(0xff00,	extractSegmentFromPageID(0xff00000000000000ULL));
	EXPECT_EQ(0xcafe,	extractSegmentFromPageID(0xcafe000000000000ULL));

	EXPECT_EQ(1,		pageIDFromSegmentID(extractSegmentFromPageID(1)));
	EXPECT_EQ(0xcafe,	pageIDFromSegmentID(extractSegmentFromPageID(0xcafe)));
}

TEST(UtilTest, extractPageFromPageID) {
	EXPECT_EQ(1,						extractPageFromPageID(1));
	EXPECT_EQ(0,						extractPageFromPageID(0xcafe000000000000ULL));
	EXPECT_EQ(0x0000f00000000000ULL,	extractPageFromPageID(0x0000f00000000000ULL));
	EXPECT_EQ(0x0000000000000001ULL,	extractPageFromPageID(0x000f000000000001ULL));
	EXPECT_EQ(0x0000f00000000000ULL,	extractPageFromPageID(0x000ff00000000000ULL));
}

TEST(UtilTest, extractSlotIDFromTID) {
	EXPECT_EQ(1,		extractSlotIDFromTID(1));
	EXPECT_EQ(0,		extractSlotIDFromTID(0xcafecafecafe0000ULL));
	EXPECT_EQ(0xf000,	extractSlotIDFromTID(0xcafecafecafef000ULL));
	EXPECT_EQ(0xbeef,	extractSlotIDFromTID(0x000000000000beefULL));
	EXPECT_EQ(0xbeef,	extractSlotIDFromTID(0xcafecafecafebeefULL));
}

TEST(UtilTest, extractPageIDFromTID) {
	EXPECT_EQ(0,						extractPageIDFromTID(0x000000000000cafeULL));
	EXPECT_EQ(0x0000cafecafecafeULL,	extractPageIDFromTID(0xcafecafecafe0000ULL));
	EXPECT_EQ(0x0000cafecafecafeULL,	extractPageIDFromTID(0xcafecafecafebeefULL));
}

} // namespace
