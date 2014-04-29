#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#include "buffermanager.h"
#include "testutil.h"

using namespace std;

namespace {

TEST(BufferManagerTest, InitAndDestructEmpty) {
	testutil::Timeout timer(200);
	BufferManager b(0);
	EXPECT_EQ(0, b.size());
	timer.finished();
}

TEST(BufferManagerTest, InitAndDestruct) {
	testutil::Timeout timer(200);
	BufferManager b(1);
	EXPECT_EQ(1, b.size());
	timer.finished();
}

TEST(BufferManagerTest, CanFix) {
	testutil::Timeout timer(200);

	BufferManager b(1);
	BufferFrame &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());
	b.unfixPage(f, false);

	timer.finished();
}

TEST(BufferManagerTest, CanFixExclusive) {
	testutil::Timeout timer(200);

	BufferManager b(1);
	BufferFrame &f2 = b.fixPage(2, true);
	EXPECT_EQ(2, f2.pageId());
	b.unfixPage(f2, false);

	timer.finished();
}

TEST(BufferManagerTest, CanFixUnfixDirty) {
	testutil::Timeout timer(200);

	BufferManager b(1);
	BufferFrame &f2 = b.fixPage(2, true);
	EXPECT_EQ(2, f2.pageId());
	b.unfixPage(f2, true);

	timer.finished();
}

TEST(BufferManagerTest, CanFixUnfixTwice) {
	testutil::Timeout timer(400);

	BufferManager b(2);
	BufferFrame &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());

	BufferFrame &f2 = b.fixPage(2, false);
	EXPECT_EQ(2, f2.pageId());

	b.unfixPage(f, false);
	b.unfixPage(f2, false);

	timer.finished();
}

TEST(BufferManagerTest, CanWaitForFreeFrame) {
	testutil::Timeout timer(400);

	BufferManager b(1);
	auto &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());

	thread giveBack([] (BufferManager *b, BufferFrame *f){
		this_thread::sleep_for(chrono::milliseconds(100));
		b->unfixPage(*f, false);
	}, &b, &f);

	auto &f2 = b.fixPage(2, false);
	EXPECT_EQ(2, f.pageId());
	b.unfixPage(f2, false);

	giveBack.join();

	timer.finished();
}

typedef struct {
	int a;
	int b;
	int c;
} Data;

TEST(BufferManagerTest, Basic) {
	srand(time(0));
	int page = rand() % 100;
	int a = rand(), b = rand(), c = rand();

	testutil::Timeout timer(400);

	BufferManager bm(16);
	{
		BufferFrame &f = bm.fixPage(page, true);

		Data *data = reinterpret_cast<Data*>(f.getData());
		data->a = a; data->b = b; data->c = c;

		bm.unfixPage(f, true);
	} {
		BufferFrame &f = bm.fixPage(page, false);

		Data *data = reinterpret_cast<Data*>(f.getData());
		EXPECT_EQ(a, data->a);
		EXPECT_EQ(b, data->b);
		EXPECT_EQ(c, data->c);

		bm.unfixPage(f, false);
	}

	timer.finished();
}

} // namespace
