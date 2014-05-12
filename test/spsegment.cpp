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

	SPSegmentTest() {
		srand(time(0));
	}

	virtual void SetUp() {
		fm = new FileManager("test_data");
		bm = shared_ptr<BufferManager>(new BufferManager(32, fm));
	}

	virtual void TearDown() {
		bm = nullptr;
		delete fm; // will crash if anyone still has this shared ptr
	}

	char randChar() {
		return 'a' + (rand() % ('z'-'a'));
	}

	void fillTest(uint64_t fillSize, bool unload, bool unloadBuffer, bool unloadFile);

	FileManager *fm;
	shared_ptr<BufferManager> bm;

};

TEST_F(SPSegmentTest, putPageIDInSegment) {
	SPSegment segment(0xcafe, bm, false);
	EXPECT_EQ(0xcafe00000000beefULL, segment.putSegmentInPageID(0xbeef));
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

	string s(randChar(), 1);

	string s2(s);
	Record r(s2.length(), s2.c_str());
	TID tid = segment.insert(r);
	EXPECT_NE(0, tid);

	Record r2 = segment.lookup(tid);
	string s3(r2.data(), r2.len());
	EXPECT_EQ(s, s3);
}

TEST_F(SPSegmentTest, UseWithUnload) {
	string s(randChar(), 1);

	TID tid;

	{
		SPSegment segment(1, bm, true);
		ASSERT_EQ(1, segment.segment());

		string s2(s);
		Record r(s2.length(), s2.c_str());
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

TEST_F(SPSegmentTest, Remove) {
	SPSegment segment(1, bm, true);
	ASSERT_EQ(1, segment.segment());

	string s(randChar(), 1);

	string s2(s);
	Record r(s2.length(), s2.c_str());
	TID tid = segment.insert(r);
	EXPECT_NE(0, tid);

	Record r2 = segment.lookup(tid);
	string s3(r2.data(), r2.len());
	EXPECT_EQ(s, s3);

	segment.remove(tid);
	Record r3 = segment.lookup(tid);
	EXPECT_EQ(0, r3.len());
}

TEST_F(SPSegmentTest, RemoveTwice) {
	SPSegment segment(1, bm, true);
	ASSERT_EQ(1, segment.segment());

	string s(randChar(), 1);

	string s2(s);
	Record r(s2.length(), s2.c_str());
	TID tid = segment.insert(r);
	EXPECT_NE(0, tid);

	Record r2 = segment.lookup(tid);
	string s3(r2.data(), r2.len());
	EXPECT_EQ(s, s3);

	segment.remove(tid);
	Record r3 = segment.lookup(tid);
	EXPECT_EQ(0, r3.len());

	segment.remove(tid);
	Record r4 = segment.lookup(tid);
	EXPECT_EQ(0, r4.len());
}


TEST_F(SPSegmentTest, FillPage) {
	fillTest(2 * PAGE_SIZE, false, false, false);
}

TEST_F(SPSegmentTest, FillPages) {
	fillTest(2 * PAGE_SIZE, false, false, false);
	fillTest(2 * PAGE_SIZE, false, false, false);
	fillTest(2 * PAGE_SIZE, false, false, false);
	fillTest(2 * PAGE_SIZE, false, false, false);
}

TEST_F(SPSegmentTest, FillPageWithUnload) {
	fillTest(2 * PAGE_SIZE, true, false, false);
}

TEST_F(SPSegmentTest, FillPageWithUnloadBuffer) {
	fillTest(2 * PAGE_SIZE, true, true, false);
}

TEST_F(SPSegmentTest, FillPageWithUnloadFile) {
	fillTest(2 * PAGE_SIZE, true, true, true);
}

TEST_F(SPSegmentTest, Fill8Pages) {
	fillTest(8 * PAGE_SIZE, false, false, false);
}

TEST_F(SPSegmentTest, Fill8PagesWithUnload) {
	fillTest(8 * PAGE_SIZE, true, false, false);
}

TEST_F(SPSegmentTest, Fill8PagesWithUnloadBuffer) {
	fillTest(8 * PAGE_SIZE, true, true, false);
}

TEST_F(SPSegmentTest, Fill8PagesWithUnloadFile) {
	fillTest(8 * PAGE_SIZE, true, true, true);
}

TEST_F(SPSegmentTest, DISABLED_Fill64Pages) {
	fillTest(64 * PAGE_SIZE, false, false, false);
}

TEST_F(SPSegmentTest, DISABLED_Fill64PagesWithUnload) {
	fillTest(64 * PAGE_SIZE, true, false, false);
}

TEST_F(SPSegmentTest, DISABLED_Fill64PagesWithUnloadBuffer) {
	fillTest(64 * PAGE_SIZE, true, true, false);
}

TEST_F(SPSegmentTest, DISABLED_Fill64PagesWithUnloadFile) {
	fillTest(64 * PAGE_SIZE, true, true, true);
}

void SPSegmentTest::fillTest(uint64_t fillSize, bool unload, bool unloadBuffer,
		bool unloadFile) {
	unordered_map<TID, string> strings;
	SPSegment *segment = new SPSegment(1, bm, true);
	uint64_t recordSize = 64;
	for (uint64_t bytesFilled = 0; bytesFilled < fillSize;
			bytesFilled += recordSize) {
		string s;
		s.reserve(recordSize);
		for (uint64_t i=0; i< recordSize; i++) {
			s.push_back(randChar());
		}
		Record r(s.length(), s.c_str());
		TID tid = segment->insert(r);
		EXPECT_NE(0, tid);
#ifndef SILENT
		cerr << "insert "<<hex<<tid<< ": "<<s<<endl;
#endif
		EXPECT_EQ(strings.end(), strings.find(tid)) << "TID given twice";
		strings.insert(pair<TID, string>(tid, s));
	}

	if (unload || unloadBuffer ||unloadFile) {
		delete segment;
		if (unloadBuffer) {
			bm = nullptr;
			if (unloadFile) {
				delete fm;
				fm = new FileManager("test_data");
			}
			bm = shared_ptr<BufferManager>(new BufferManager(32, fm));
		}
		segment = new SPSegment(1, bm, false);
	}

	for (pair<TID, string> t : strings) {
#ifndef SILENT
		cerr << "lookup "<<hex<<t.first<< ": "<<t.second<<endl;
#endif
		Record r = segment->lookup(t.first);
		string s(r.data(), r.len());
		EXPECT_EQ(t.second, s);
	}
	delete segment;
}

} // namespace
