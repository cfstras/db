#include <gtest/gtest.h>
#include "testutil.h"

#include "metasegment.h"

using namespace std;

namespace {

class MetaSegmentTest : public ::testing::Test {
protected:
	void SetUp() {
		srand(time(0));
		fm = shared_ptr<FileManager>(new FileManager("test_data"));
		bm = shared_ptr<BufferManager>(new BufferManager(1, fm));
	}

	void putGet(bool unload, bool unloadFiles) {
		MetaSegment seg(bm, true);

		MetaDatum m;
		m.name = testutil::randString(20);
		m.segmentID = rand() % 20 + 10;
		m.numPages = rand();
		m.type = SegmentType::SPSegment;
		m.payload = testutil::randString(200);

		ASSERT_EQ(true, seg.put(m));

		if (unload || unloadFiles) {
			if (unloadFiles) {
				fm.reset(new FileManager("test_data"));
			}
			bm.reset(new BufferManager(1, fm));
		}
		MetaDatum d = seg.get(m.name);
		EXPECT_EQ(m.name, d.name);
		EXPECT_EQ(m.segmentID, d.segmentID);
		EXPECT_EQ(m.numPages, d.numPages);
		EXPECT_EQ(m.type, d.type);
		EXPECT_EQ(m.payload, d.payload);
	}

	std::shared_ptr<BufferManager> bm;
	std::shared_ptr<FileManager> fm;
};

TEST_F(MetaSegmentTest, Init) {
	MetaSegment seg(bm, true);
}

TEST_F(MetaSegmentTest, Construct) {
	MetaSegment seg(bm);
}

TEST_F(MetaSegmentTest, InitSame) {
	MetaDatum m = {"blah", 1, 1, SegmentType::SPSegment, "bluh"};
	{
		MetaSegment seg(bm, true);
		ASSERT_EQ(true, seg.put(m));
	}
	{
		MetaSegment seg(bm, true);
		MetaDatum d = seg.get(m.name);
		EXPECT_NE(d.name, m.name);
		EXPECT_NE(d.segmentID, m.segmentID);
		EXPECT_NE(d.numPages, m.numPages);
		EXPECT_NE(d.type, m.type);
		EXPECT_NE(d.payload, m.payload);
	}
}

TEST_F(MetaSegmentTest, PutGet) {
	putGet(false, false);
}

TEST_F(MetaSegmentTest, PutGetBuffer) {
	putGet(true, false);
}

TEST_F(MetaSegmentTest, PutGetFile) {
	putGet(true, true);
}

} // namespace
