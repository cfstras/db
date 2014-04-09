#include "sort.h"

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <queue>

#include "util.h"

using namespace std;

typedef uint64_t T;

namespace {

struct PairCompare {
	bool operator()(pair<T,size_t> const & o1, pair<T,size_t> const & o2) {
		return o1.first < o2.first;
	}
};

class Sorter {
public:
	Sorter(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);

private:
	void stepSort(int fdInput);
	void stepMerge(int fdOutput);
	void fillQueue(size_t chunkInd, size_t bufferSize);

	vector<uint64_t> chunkPositions;

	// queue contains pairs of (number, buffer_index)
	priority_queue<pair<T,size_t>, vector<pair<T,size_t>>, PairCompare> queue;

	uint64_t memSize;
	uint64_t size;
	size_t chunkLength;
	vector<T> buffer;

	size_t numChunks;

	int fdTemp;
};

Sorter::Sorter(int fdInput, uint64_t _size, int fdOutput, uint64_t _memSize) :
		chunkPositions(),
		memSize(_memSize),
		size(_size),
		chunkLength(memSize / sizeof(T)),
		buffer(chunkLength) {

	//TODO the length should be less, since our sorting algorithm and prio-queue
	// take up space, too.

	// create temp file
	char tempTemplate[] ="db-externalsortXXXXXX";
	fdTemp = mkstemp(tempTemplate);
	if (fdTemp == -1) {
		util::checkReturn("creating chunk tempfile", errno);
	}

	size_t tempFileSize = size * sizeof(T);
	util::checkReturn("allocating chunk tempfile",
		posix_fallocate(fdTemp, 0, static_cast<off_t>(tempFileSize)));

	stepSort(fdInput);
	stepMerge(fdOutput);

	util::checkReturn("closing temp file", close(fdTemp));
}

void Sorter::stepSort(int fdInput) {
	// save the positions (as index) of the chunk starting points
	chunkPositions.reserve(size * sizeof(T) / memSize);

	// sort chunks
	ssize_t bRead;
	uint64_t elementsSorted = 0;
	size_t maxChunkBytes = chunkLength*sizeof(T);
	while ((bRead = read(fdInput, &buffer[0], maxChunkBytes)) > 0 &&
			elementsSorted < size) {
		auto bytesRead = static_cast<size_t>(bRead);

		uint64_t elementsRead = static_cast<uint64_t>(bytesRead) / sizeof(T);

		auto begin = buffer.begin();
		auto end = buffer.begin() + static_cast<int64_t>(elementsRead);
		sort(begin, end);

		// write chunk to temp file
		auto written = write(fdTemp, &buffer[0], bytesRead);
		if (written < 0) {
			util::checkReturn("writing chunk to tempfile", errno);
		}

		chunkPositions.push_back(elementsSorted);
		numChunks++;
		elementsSorted += elementsRead;
	}
	if (bRead == -1) {
		util::checkReturn("reading from input", errno);
	}
}

void Sorter::stepMerge(int fdOutput) {
	// now merge.

	// get some space in the output file
	off_t offset = lseek(fdOutput, 0, SEEK_CUR);
	util::checkReturn("allocating output space",
		posix_fallocate(fdOutput, offset, static_cast<off_t>(size * sizeof(T))));

	// input/output buffer size
	// one space left for output buffer
	size_t bufferSize = memSize / ( 1 + chunkPositions.size());
	auto bufferLen = bufferSize / sizeof(T);

	// smaller buffer for bigger prio queue
	buffer.resize(bufferLen);
	buffer.shrink_to_fit();

	// fill up the queue
	for (size_t chunkInd=0; chunkInd < numChunks; chunkInd++) {
		fillQueue(chunkInd, bufferSize);
	}

	buffer.resize(0);
	size_t elementsSorted = 0;
	while (elementsSorted < size) {
		// pull from queue and write out
		while (!queue.empty()) {
			auto el = queue.top();
			queue.pop();
			cout << el.first << endl;
			buffer.push_back(el.first);
			elementsSorted++;
			if (buffer.size() == bufferLen) {
				auto written = write(fdOutput, &buffer[0], bufferSize);
				if (written == -1) {
					util::checkReturn("writing output", errno);
				}
				buffer.resize(0);
				cout << endl;
			}

			auto chunkInd = el.second;
			// if chunk still has some left
			if (chunkInd+1 < numChunks && chunkPositions[chunkInd] <= chunkPositions[chunkInd+1
				|| chunkPositions[chunkInd] <= size]) {
				fillQueue(chunkInd, bufferSize);
			}
		}
	}
	// flush
	auto written = write(fdOutput, &buffer[0], buffer.size() * sizeof(T));
	if (written == -1) {
		util::checkReturn("writing output", errno);
	}
}

void Sorter::fillQueue(size_t chunkInd, size_t bufferSize) {
	size_t chunkLen;
	if (chunkInd+1 < numChunks) {
		chunkLen = chunkPositions[chunkInd+1] - chunkPositions[chunkInd];
	} else {
		chunkLen = size - chunkPositions[chunkInd];
	}
	chunkLen = min(bufferSize / sizeof(T), chunkLen);
	auto bytesToRead = chunkLen * sizeof(T);
	auto bytesRead = pread(fdTemp, &buffer[0], bytesToRead,
			static_cast<int64_t>(chunkPositions[chunkInd] * sizeof(T)));
	if (bytesRead == -1) {
		util::checkReturn("reading from temp file", errno);
	}
	for (size_t i = 0; i < chunkLen; i++) {
		queue.emplace(pair<T,size_t>(buffer[i], chunkInd));
	}

	// move starting position
	chunkPositions[chunkInd] += chunkLen;
}

} // namespace

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
	Sorter(fdInput, size, fdOutput, memSize);
}
