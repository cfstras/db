#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include <chrono>
#include <thread>

#include <testutil.h>

using namespace std;

namespace {

void run(int timeout, int sleep) {
	testutil::Timeout timer(timeout);
	this_thread::sleep_for(chrono::milliseconds(sleep));
	timer.finished();
}

TEST(TestUtilTest, Timer) {
	run(100, 20);
}

TEST(TestUtilTest, TimerFail) {
	EXPECT_FATAL_FAILURE_ON_ALL_THREADS(run(100, 1000), "Timeout");
}

} // namespace
