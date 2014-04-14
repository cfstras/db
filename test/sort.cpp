#include <gtest/gtest.h>

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "sort.h"

using namespace std;

namespace {

class SortTest : public ::testing::Test {
protected:

	SortTest() : generatorBin( "datagenerator/gen") {
		char str[] = "externalsort-test-XXXXXX";
		fdTempUnsorted = mkstemp(str);
		unsortedFile = str;
		close(fdTempUnsorted);
		open(fdTempUnsorted);
		unlink(unsortedFile.c_str());

		char strS[] = "externalsort-test-XXXXXX";
		fdTempSorted = mkstemp(strS);
		sortedFile = strS;
		close(fdTempSorted);
		open(fdTempSorted);
		unlink(sortedFile.c_str());
	}

	virtual void TearDown() {
		close(fdTempUnSorted);
		close(fdTempSorted)
	}

	int generate(uint64_t num) {
		string cmd = generatorBin + " " + unsortedFile + " " + to_string(num);
		return system(cmd.c_str());
	}

	void checkOrder(int fdInput, uint64_t size) {
		size_t bufSize = 1024;
		vector<uint64_t> buf(bufSize);
		uint64_t bufPos = 0, num = 0, maxNum = 1, last = 0;
		for (;num < size; num++) {
			if (++bufPos == bufSize || num == 0) {
				bufPos = 0;
				auto bytes = read(fdInput, &buf[0], sizeof(uint64_t) * bufSize);
				ASSERT_NE(-1, bytes) << "error reading test output: " << strerror(errno);
				if (bytes == 0) {
					break;
				}
			}
			uint64_t val = buf[bufPos];
			EXPECT_GE(val, last) << "integers not sorted, at pos " << num;
			last = val;
		}
		EXPECT_EQ(size, num) << "size wrong!";
	}

	void sortTest(uint64_t size, uint64_t memSize) {
		ASSERT_EQ(0, generate(size)) << "DataGenerator failed to run";

		int fdInput = open(unsortedFile.c_str(), O_RDONLY);
		int fdOutput = open(sortedFile.c_str(), O_RDWR | O_TRUNC);
		ASSERT_NE(-1, fdInput) << "Opening file " << unsortedFile << " failed: " << strerror(errno);
		ASSERT_NE(-1, fdOutput) << "Opening file " << sortedFile << " failed: " << strerror(errno);
		externalSort(fdInput, size, fdOutput, memSize);
		close(fdInput);

		ASSERT_NE(-1, lseek(fdOutput, 0, SEEK_SET)) << "error reading test output: " << strerror(errno);
		checkOrder(fdOutput, size);
		close(fdOutput);
	}

	string unsortedFile, sortedFile;
	string generatorBin;

	int fdTempUnsorted, fdTempSorted;
};

TEST_F(SortTest, Generator) {
	EXPECT_EQ(0, generate(1)) << "DataGenerator failed to run";
}

//TODO does not leave behind tempfiles

TEST_F(SortTest, SortSmall) {
	sortTest(64, 128 * 8);
}

TEST_F(SortTest, DoesNotCloseFDs) {
	ASSERT_EQ(0, generate(64)) << "DataGenerator failed to run";

	int fdInput = open(unsortedFile.c_str(), O_RDONLY);
	int fdOutput = open(sortedFile.c_str(), O_RDWR | O_TRUNC);
	ASSERT_NE(-1, fdInput) << "Opening file " << unsortedFile << " failed: " << strerror(errno);
	ASSERT_NE(-1, fdOutput) << "Opening file " << sortedFile << " failed: " << strerror(errno);
	externalSort(fdInput, 64, fdOutput, 128 * 8);
	EXPECT_NE(-1, close(fdInput)) <<
		"output file descriptor was closed by sort: " << strerror(errno);
	EXPECT_NE(-1, close(fdOutput)) <<
		"output file descriptor was closed by sort: " << strerror(errno);
}

TEST_F(SortTest, SortBig) {
	sortTest(2048, 512 * 8);
}

TEST_F(SortTest, SortMany) {
	sortTest(2048, 32 * 8);
}

TEST_F(SortTest, SortNothing) {
	sortTest(0, 32 * 8);
}

TEST_F(SortTest, SortHuge) {
	sortTest(1024 * 1024 * 1024 * 5 / 8, // 5GiB data
		1024 * 1024 * 1024); // 1GiB Memory
}

} // namespace
