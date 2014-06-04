#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>

#include "buffermanager.h"
#include "filemanager.h"
#include "testutil.h"

using namespace std;
using namespace testutil;

namespace {

TEST(BufferManagerTest, InitAndDestructEmpty) {
	Timeout *timer = new Timeout(200);
	BufferManager b(0);
	EXPECT_EQ(0, b.size());
	timer->finished();
	delete timer;
}

TEST(BufferManagerTest, InitAndDestruct) {
	Timeout *timer = new Timeout(200);
	BufferManager b(1);
	EXPECT_EQ(1, b.size());
	timer->finished();
	delete timer;
}


TEST(BufferManagerTest, InitAndDestructAlt) {
	Timeout *timer = new Timeout(200);

	shared_ptr<FileManager> f(new FileManager("test_data"));
	BufferManager b(1, f);
	EXPECT_EQ(1, b.size());

	timer->finished();
	delete timer;
}

TEST(BufferManagerTest, CanFix) {
	Timeout *timer = new Timeout(200);

	BufferManager b(1);
	BufferFrame &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());
	b.unfixPage(f, false);

	timer->finished();
	delete timer;
}

TEST(BufferManagerTest, CanFixExclusive) {
	Timeout *timer = new Timeout(200);

	BufferManager b(1);
	BufferFrame &f2 = b.fixPage(2, true);
	EXPECT_EQ(2, f2.pageId());
	b.unfixPage(f2, false);

	timer->finished();
	delete timer;
}

TEST(BufferManagerTest, CanFixUnfixDirty) {
	Timeout *timer = new Timeout(200);

	BufferManager b(1);
	BufferFrame &f2 = b.fixPage(2, true);
	EXPECT_EQ(2, f2.pageId());
	b.unfixPage(f2, true);

	timer->finished();
	delete timer;
}

TEST(BufferManagerTest, CanFixUnfixTwice) {
	Timeout *timer = new Timeout(400);

	BufferManager b(2);
	BufferFrame &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());

	BufferFrame &f2 = b.fixPage(2, false);
	EXPECT_EQ(2, f2.pageId());

	b.unfixPage(f, false);
	b.unfixPage(f2, false);

	timer->finished();
	delete timer;
}

TEST(BufferManagerTest, CanWaitForFreeFrame) {
	Timeout *timer = new Timeout(400);

	BufferManager b(1);
	auto &f = b.fixPage(1, false);
	EXPECT_EQ(1, f.pageId());

	thread *giveBack = new thread([&] (){
		this_thread::sleep_for(chrono::milliseconds(100));
		b.unfixPage(f, false);
	});

	auto &f2 = b.fixPage(2, false);
	EXPECT_EQ(2, f2.pageId());
	b.unfixPage(f2, false);

	giveBack->join();
	delete giveBack;
	timer->finished();
	delete timer;
}

typedef struct {
	int a;
	int b;
	int c;
} Data;

void basicTest(bool withUnload, bool withFileManager) {
	srand(time(0));
	int page = (rand() % 100) | util::pageIDFromSegmentID(2);
	int a = rand(), b = rand(), c = rand();

	Timeout *timer = new Timeout(400);

	shared_ptr<FileManager> fm(new FileManager("test_data"));
	shared_ptr<BufferManager> bm(new BufferManager(16, fm));
	{
		BufferFrame &f = bm->fixPage(page, true);

		Data *data = reinterpret_cast<Data*>(f.getData());
		data->a = a; data->b = b; data->c = c;

		bm->unfixPage(f, true);
	}
	if (withUnload || withFileManager) {
		if (withFileManager) {
			fm.reset(new FileManager("test_data"));
		}
		bm.reset(new BufferManager(16, fm));
	}
	{
		BufferFrame &f = bm->fixPage(page, false);

		Data *data = reinterpret_cast<Data*>(f.getData());
		EXPECT_EQ(a, data->a);
		EXPECT_EQ(b, data->b);
		EXPECT_EQ(c, data->c);

		bm->unfixPage(f, false);
	}
	fm.reset((FileManager*)nullptr);
	bm.reset((BufferManager*)nullptr);

	timer->finished();
	delete timer;
}

TEST(BufferManagerTest, Basic) {
	basicTest(false, false);
}

TEST(BufferManagerTest, BasicWithUnload) {
	basicTest(true, false);
}

TEST(BufferManagerTest, BasicWithFileUnload) {
	basicTest(true, true);
}


} // namespace
