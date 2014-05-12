#pragma once

#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

#include "util.h"
#include "buffermanager.h"

namespace testutil {

class Timeout {
 public:
	Timeout(uint64_t msecs) :
	maxTime(std::chrono::system_clock::now() + std::chrono::milliseconds(msecs)),
	finished_(false), mutex() {
		mutex.lock();
		timer = std::thread(&Timeout::join, this);
	}

	void finished() {
		finished_ = true;
		mutex.unlock();
		timer.join();
	}

 private:
	DISALLOW_COPY_AND_ASSIGN(Timeout);

	void join() {
		while (!finished_ && std::chrono::system_clock::now() < maxTime) {
			auto waitFor = maxTime - std::chrono::system_clock::now();
			//std::cerr << "waiting for "<< std::chrono::duration_cast<std::chrono::milliseconds>(waitFor).count() << std::endl;
			bool ret = mutex.try_lock_for(waitFor);
			if (ret) {
				mutex.unlock();
				break;
			}
		}
		if (!finished_) {
			FAIL() << "Timeout";
		}
	}

	std::chrono::time_point<std::chrono::system_clock> maxTime;
	std::atomic<bool> finished_;
	std::timed_mutex mutex;
	std::thread timer;
};

} // namespace
