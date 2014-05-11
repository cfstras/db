#include <gtest/gtest.h>

#include "spsegment.h"

#include <memory>
#include <unordered_map>
#include <iostream>
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

	void fillTest(uint64_t fillSize, bool unload);

	FileManager fm;
	shared_ptr<BufferManager> bm;

};

TEST_F(SPSegmentTest, putPageIDInSegment) {
	SPSegment segment(1, bm, true);
	EXPECT_EQ(0x0001000000000001ULL, segment.putSegmentInPageID(1));
}

TEST_F(SPSegmentTest, isPageInThisSegment) {
	SPSegment segment(2, bm, true);
	EXPECT_EQ(true, segment.isPageInThisSegment(0));
	EXPECT_EQ(false, segment.isPageInThisSegment(1));
}

TEST_F(SPSegmentTest, Construct) {
	SPSegment segment(1, bm, true);
	EXPECT_EQ(1, segment.segment());
}

TEST_F(SPSegmentTest, Load) {
	SPSegment segment(1, bm, false);
	EXPECT_EQ(1, segment.segment());
}

TEST_F(SPSegmentTest, Use) {
	SPSegment segment(1, bm, true);
	ASSERT_EQ(1, segment.segment());

	string s(" ");
	s[0] = 'a' + (rand() % ('z'-'a'));

	Record r(s.length(), s.c_str());
	TID tid = segment.insert(r);
	EXPECT_NE(0, tid);

	Record r2 = segment.lookup(tid);
	string s3(r2.data(), r2.len());
	EXPECT_EQ(s, s3);
}

TEST_F(SPSegmentTest, UseWithUnload) {
	string s(" ");
	s[0] = 'a' + (rand() % ('z'-'a'));
	string s2(s);
	TID tid;

	{
		SPSegment segment(1, bm, true);
		ASSERT_EQ(1, segment.segment());

		Record r(s.length(), s2.c_str());
		tid = segment.insert(r);
		EXPECT_NE(0, tid);
	}

	SetUp(); // new BufferManager
	{
		SPSegment segment(1, bm);
		Record r2 = segment.lookup(tid);
		string s3(r2.data(), r2.len());
		EXPECT_EQ(s, s3);
	}
}

TEST_F(SPSegmentTest, FillPage) {
	fillTest(2 * PAGE_SIZE, false);
}

TEST_F(SPSegmentTest, FillPageWithUnload) {
	fillTest(2 * PAGE_SIZE, true);
}

TEST_F(SPSegmentTest, Fill64Pages) {
	fillTest(64 * PAGE_SIZE, false);
}

TEST_F(SPSegmentTest, Fill64PagesWithUnload) {
	fillTest(64 * PAGE_SIZE, true);
}

void SPSegmentTest::fillTest(uint64_t fillSize, bool unload) {
	unordered_map<TID, string> strings;
	SPSegment *segment = new SPSegment(1, bm, true);
	uint64_t recordSize = 64;
	for (uint64_t bytesFilled = 0; bytesFilled < fillSize;
			bytesFilled += recordSize) {
		string s;
		s.reserve(recordSize);
		for (uint64_t i=0; i< recordSize; i++) {
			s.push_back('a' + (rand() % ('z'-'a')));
		}
		Record r(s.length(), s.c_str());
		TID tid = segment->insert(r);
		EXPECT_NE(0, tid);
#ifndef SILENT
		cerr << "insert "<<hex<<tid<< ": "<<s<<endl;
#endif
		strings.insert(pair<TID, string>(tid, s));
	}

	if (unload) {
		SetUp();
		segment = new SPSegment(1, bm, false);
	}

	for (pair<TID, string> t : strings) {
		Record r = segment->lookup(t.first);
		string s(r.data(), r.len());
		EXPECT_EQ(t.second, s);
	}
}

} // namespace
