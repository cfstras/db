#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#include "buffermanager.h"

using namespace std;

namespace {

bool doTimeout;

void timeout() {
	doTimeout = true;
	this_thread::sleep_for(chrono::milliseconds(100));
	if (!doTimeout) {
		FAIL() << "Timeout";
	}
}

TEST(BufferManagerTest, InitAndDestruct) {
	BufferManager b(1);
	EXPECT_EQ(1, b.size());
}

TEST(BufferManagerTest, CanFix) {
	thread timer(timeout);

	BufferManager b(1);
	BufferFrame &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());
	b.unfixPage(f, false);

	doTimeout = false;
	timer.join();
}

TEST(BufferManagerTest, CanFixExclusive) {
	thread timer(timeout);

	BufferManager b(1);
	BufferFrame &f2 = b.fixPage(2, true);
	EXPECT_EQ(2, f2.pageId());
	b.unfixPage(f2, false);

	doTimeout = false;
	timer.join();
}

TEST(BufferManagerTest, CanFixUnfixDirty) {
	thread timer(timeout);

	BufferManager b(1);
	BufferFrame &f2 = b.fixPage(2, true);
	EXPECT_EQ(2, f2.pageId());
	b.unfixPage(f2, true);

	doTimeout = false;
	timer.join();
}

TEST(BufferManagerTest, CanFixUnfixTwice) {
	thread timer(timeout);

	BufferManager b(2);
	BufferFrame &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());

	BufferFrame &f2 = b.fixPage(2, false);
	EXPECT_EQ(2, f.pageId());

	b.unfixPage(f, false);
	b.unfixPage(f2, false);

	doTimeout = false;
	timer.join();
}

typedef struct {
	int a;
	int b;
	int c;
} Data;

TEST(BufferManagerTest, Basic) {
	srand(time(0));
	int a = rand(), b = rand(), c = rand();

	thread timer(timeout);

	BufferManager bm(16);
	{
		BufferFrame &f = bm.fixPage(1, true);

		Data *data = reinterpret_cast<Data*>(f.getData());
		data->a = a; data->b = b; data->c = c;

		bm.unfixPage(f, true);
	} {
		BufferFrame &f = bm.fixPage(1, false);

		Data *data = reinterpret_cast<Data*>(f.getData());
		EXPECT_EQ(a, data->a);
		EXPECT_EQ(b, data->b);
		EXPECT_EQ(c, data->c);

		bm.unfixPage(f, false);
	}

	doTimeout = false;
	timer.join();
}

} // namespace
