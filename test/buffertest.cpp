#include <gtest/gtest.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "buffermanager.h"
#include "testutil.h"

using namespace std;

namespace {

BufferManager* bm;
unsigned pagesOnDisk;
unsigned pagesInRAM;
unsigned threadCount;
unsigned* threadSeed;
unsigned iterationCount;
volatile bool stop=false;

unsigned randomPage(unsigned threadNum) {
	// pseudo-gaussian, causes skewed access pattern
	unsigned page=0;
	for (unsigned  i=0; i<20; i++)
		page+=rand_r(&threadSeed[threadNum])%pagesOnDisk;
	return page/20;
}

static void scan1() {
	// scan all pages and check if the counters are not decreasing
	unsigned counters[pagesOnDisk];
	for (unsigned i=0; i<pagesOnDisk; i++)
		counters[i]=0;

	while (!stop) {
		unsigned start = random()%(pagesOnDisk-10);
		for (unsigned page=start; page<start+10; page++) {
			BufferFrame &bf = bm->fixPage(page, false);
			unsigned newcount = reinterpret_cast<unsigned*>(bf.getData())[0];
			EXPECT_LE(counters[page], newcount);
			counters[page]=newcount;
			bm->unfixPage(bf, false);
		}
	}
}

static void* scan(void *arg) {
	scan1();
	return NULL;
}

static void* readWrite(void *arg) {
	// read or write random pages
	uintptr_t threadNum = reinterpret_cast<uintptr_t>(arg);

	int progress = 0, pn;
	uintptr_t count = 0;
	for (unsigned i=0; i<iterationCount/threadCount && !stop; i++) {
		bool isWrite = rand_r(&threadSeed[threadNum])%128<10;
		BufferFrame &bf = bm->fixPage(randomPage(threadNum), isWrite);

		if (isWrite) {
			count++;
			reinterpret_cast<unsigned*>(bf.getData())[0]++;
		}
		bm->unfixPage(bf, isWrite);

		pn = (i*77 / (iterationCount/threadCount));
		if (pn > progress) {
			string a(pn - progress,to_string(threadNum)[0]);
			progress = pn;
			cerr << a;
		}
	}
	cerr << endl;

	return reinterpret_cast<void*>(count);
}

void test(unsigned pagesDisk, unsigned pagesRam, unsigned nThreads, unsigned iterations) {
	pagesOnDisk = pagesDisk;
	pagesInRAM = pagesRam;
	threadCount = nThreads;
	iterationCount = iterations;

	ASSERT_GE(pagesDisk, 10);
	ASSERT_GE(pagesRam, 10);

	threadSeed = new unsigned[threadCount];
	for (unsigned i=0; i<threadCount; i++)
		threadSeed[i] = i*97134;

	bm = new BufferManager(pagesInRAM);

	for (unsigned i=0; i<nThreads; i++) {
		cerr << "[" << string(77, '-') << "]" << endl;
	}

	pthread_t threads[threadCount];
	pthread_attr_t pattr;
	pthread_attr_init(&pattr);

	// set all counters to 0
	for (unsigned i=0; i<pagesOnDisk; i++) {
		BufferFrame &bf = bm->fixPage(i, true);
		reinterpret_cast<unsigned*>(bf.getData())[0]=0;
		bm->unfixPage(bf, true);
	}

	// start scan thread
	pthread_t scanThread;
	pthread_create(&scanThread, &pattr, scan, NULL);

	// start read/write threads
	for (unsigned i=0; i<threadCount; i++)
		pthread_create(&threads[i], &pattr, readWrite, reinterpret_cast<void*>(i));

	// wait for read/write threads
	unsigned totalCount = 0;
	for (unsigned i=0; i<threadCount; i++) {
		void *ret;
		pthread_join(threads[i], &ret);
		totalCount+=reinterpret_cast<uintptr_t>(ret);
	}

	// wait for scan thread
	stop=true;
	pthread_join(scanThread, NULL);

	// restart buffer manager
	delete bm;
	bm = new BufferManager(pagesInRAM);

	// check counter
	unsigned totalCountOnDisk = 0;
	for (unsigned i=0; i<pagesOnDisk; i++) {
		BufferFrame &bf = bm->fixPage(i,false);
		totalCountOnDisk+=reinterpret_cast<unsigned*>(bf.getData())[0];
		bm->unfixPage(bf, false);
	}

	delete bm;
	EXPECT_EQ(totalCount, totalCountOnDisk) << "error: expected " << totalCount << " but got " << totalCountOnDisk;
}

/*TEST(BufferManagerTest, Concurrent) {
	thread timer(timeout);

	test(16, 16, 1, 10);

	doTimeout = false;
	timer.join();
}*/

TEST(BufferManagerTest, Concurrent2) {
	testutil::Timeout timer(10*1000);
	test(32, 32, 4, 32);
	timer.finished();
}


} // namespace
