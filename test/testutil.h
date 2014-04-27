#pragma once

#include <gtest/gtest.h>

#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <mutex>

#include "util.h"
#include "buffermanager.h"

namespace testutil {

class Timeout {
 public:
	Timeout(uint64_t msecs) :
	msecs_(msecs),
	max_time_(std::chrono::system_clock::now() + std::chrono::milliseconds(msecs)),
	finished_(false), mutex_() {
		mutex_.lock();
		timer_ = std::thread(&Timeout::join, this);
	}

	void finished() {
		finished_ = true;
		mutex_.unlock();
		timer_.join();
	}

 private:
	DISALLOW_COPY_AND_ASSIGN(Timeout);

	void join() {
		while (!finished_ && std::chrono::system_clock::now() < max_time_) {
			auto waitFor = max_time_ - std::chrono::system_clock::now();
			//std::cerr << "waiting for "<< std::chrono::duration_cast<std::chrono::milliseconds>(waitFor).count() << std::endl;
			bool ret = mutex_.try_lock_for(waitFor);
			if (ret) break;
		}
		if (!finished_) {
			FAIL() << "Timeout";
		}
	}

	uint64_t msecs_;
	std::chrono::time_point<std::chrono::system_clock> max_time_;
	bool finished_;
	std::timed_mutex mutex_;
	std::thread timer_;
};

} // namespace
