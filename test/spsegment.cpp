#include <gtest/gtest.h>

#include "spsegment.h"

#include <memory>
#include <cstdlib>
#include <ctime>

using namespace std;

namespace {

class SPSegmentTest : public ::testing::Test {
protected:

	SPSegmentTest() : fm("test_data") {
		srand(time(0));
	}

	virtual void SetUp() {
		// create a new one
		bm = shared_ptr<BufferManager>(new BufferManager(32, &fm));
	}

	virtual void TearDown() {
		// blah
	}

	FileManager fm;
	shared_ptr<BufferManager> bm;

};

TEST_F(SPSegmentTest, Construct) {
	SPSegment segment(1, bm, true);
	EXPECT_EQ(1, segment.segment());
}

TEST_F(SPSegmentTest, Load) {
	SPSegment segment(1, bm);
	EXPECT_EQ(1, segment.segment());
}

TEST_F(SPSegmentTest, Use) {
	SPSegment segment(1, bm);
	ASSERT_EQ(1, segment.segment());

	string s(" ");
	s[0] = 'a' + (rand() % ('z'-'a'));
	string s2(s);

	Record r(s.length()+1, s2.c_str());
	TID tid = segment.insert(r);
	EXPECT_NE(0, tid);

	Record r2 = segment.lookup(tid);
	EXPECT_EQ(s, r2.data());
}

} // namespace
