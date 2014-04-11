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
		return o1.first > o2.first;
	}
};

class Sorter {
public:
	Sorter(int fdInput, uint64_t size, int fdOutput, uint64_t memSize);

private:
	void stepSort(int fdInput);
	void stepMerge(int fdOutput);

	size_t fillBuffer(size_t chunkInd);
	bool fillQueue(size_t chunkInd, size_t limit);

	// chunk starting offsets
	// get moved in merge step buffer refill
	vector<uint64_t> chunkPositions;

	// queue contains pairs of (number, buffer_index)
	priority_queue<pair<T,size_t>, vector<pair<T,size_t>>, PairCompare> queue;

	uint64_t memSize;
	uint64_t size;
	size_t chunkLength;
	size_t bufferSize;

	// input buffer for sort step, output buffer for merge step
	vector<T> buffer;

	size_t numChunks;
	// input buffers for merge step
	vector<vector<T>> buffers;
	// head positions of those buffers
	vector<size_t> buffersPos;

	int fdTemp;
};

static const size_t outBufferSize = 8*1024;

Sorter::Sorter(int fdInput, uint64_t _size, int fdOutput, uint64_t _memSize) :
		chunkPositions(),
		memSize(_memSize),
		size(_size),
		chunkLength(memSize / sizeof(T)),
		bufferSize(0),
		buffer(chunkLength),
		buffers(0),
		buffersPos(0) {

	//TODO the length should be less, since our sorting algorithm and prio-queue
	// take up space, too.

	// create temp file
	char tmpFilename[] ="externalsort-XXXXXX";
	fdTemp = mkstemp(tmpFilename);
	if (fdTemp == -1) {
		util::checkReturn("creating chunk tempfile", errno);
	}

	size_t tempFileSize = size * sizeof(T);
	util::checkReturn("allocating chunk tempfile",
		posix_fallocate(fdTemp, 0, static_cast<off_t>(tempFileSize)));

	stepSort(fdInput);
	stepMerge(fdOutput);

	util::checkReturn("closing temp file", close(fdTemp));
	util::checkReturn("deleting temp file", unlink(tmpFilename));
}

void Sorter::stepSort(int fdInput) {
	// save the positions (as index) of the chunk starting points
	chunkPositions.reserve(size * sizeof(T) / memSize);

#ifdef DEBUG
	cout << "chunks:" << endl;
#endif

	// sort chunks
	ssize_t bRead;
	uint64_t elementsSorted = 0;
	size_t maxChunkBytes = chunkLength*sizeof(T);
	while ((bRead = read(fdInput, &buffer[0], maxChunkBytes)) > 0 &&
			elementsSorted < size) {
		auto bytesRead = static_cast<size_t>(bRead);

		// limit to size
		uint64_t elementsRead = static_cast<uint64_t>(bytesRead) / sizeof(T);
		elementsRead = min(elementsRead, size - elementsSorted);
		bytesRead  = elementsRead * sizeof(T);

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

#ifdef DEBUG
		for (auto it = begin; it != end; it++) {
			cout << *it << endl;
		}
		cout << endl;
#endif
	}

	if (bRead == -1) {
		util::checkReturn("reading from input", errno);
	}
}

void Sorter::stepMerge(int fdOutput) {
	// now merge.

	// ensure some space in the output file
	off_t offset = lseek(fdOutput, 0, SEEK_CUR);
	util::checkReturn("allocating output space",
		posix_fallocate(fdOutput, offset, static_cast<off_t>(size * sizeof(T))));

	// input/output buffer size
	// total needed space:
	// inputBuffers + queue + outputBuffer
	//       a * n  + a * n +  4k   = memSize       | -4k
	//       a * n * 2              = memSize - 4k  | / (2 * n )
	//       a   =    ( memSize - 4k ) / (2 * n)
	// n = numChunks
	// a = bufferSize
	//
	bufferSize = outBufferSize > memSize ? memSize : memSize - outBufferSize;

	bufferSize /= ( 2 * numChunks);
	size_t bufferLen = bufferSize / sizeof(T);

	if (bufferLen < 4) { // some sanity.
		bufferLen = 4;
		bufferSize = sizeof(T) * bufferLen;
	}

#ifdef DEBUG
	cout << "merge buffers len: " << bufferLen << endl;
#endif

	// smaller buffer for bigger prio queue
	auto outBufferLen = outBufferSize / sizeof(T);
	buffer.resize(outBufferLen);
	buffer.shrink_to_fit();
	buffer.resize(0);

	buffers.resize(numChunks);
	buffersPos.resize(numChunks);

#ifdef DEBUG
	cout << "Load:" << endl;
#endif

	// fill up the queue
	for (size_t chunkInd=0; chunkInd < numChunks; chunkInd++) {
		buffers[chunkInd].reserve(bufferLen);
		auto chunkLen = fillBuffer(chunkInd);
		fillQueue(chunkInd, chunkLen);
	}

#ifdef DEBUG
	cout << "merge:" << endl;
#endif

	// pull from queue and write out
	while (!queue.empty()) {
		auto el = queue.top();
		queue.pop();
#ifdef DEBUG
		cout << "emit "<< el.second << ": " << el.first << endl;
#endif
		buffer.push_back(el.first);
		if (buffer.size() == outBufferLen) {
			auto written = write(fdOutput, &buffer[0], outBufferSize);
			if (written == -1) {
				util::checkReturn("writing output", errno);
			}
			buffer.resize(0);
			cout << endl;
		}

		auto chunkInd = el.second;

		// queue another item if left in chunk
		fillQueue(chunkInd, 1);
	}
	// flush
	auto written = write(fdOutput, &buffer[0], buffer.size() * sizeof(T));
	if (written == -1) {
		util::checkReturn("writing output", errno);
	}
}

size_t Sorter::fillBuffer(size_t chunkInd) {
	size_t chunkLen;
	if (chunkInd+1 < numChunks) {
		chunkLen = chunkLength*(chunkInd+1) - chunkPositions[chunkInd];
	} else {
		chunkLen = size - chunkPositions[chunkInd];
	}
	chunkLen = min(bufferSize / sizeof(T), chunkLen);
	auto &buf = buffers[chunkInd];
	buf.resize(chunkLen);

	auto bytesToRead = chunkLen * sizeof(T);
	auto bytesRead = pread(fdTemp, &buf[0], bytesToRead,
			static_cast<int64_t>(chunkPositions[chunkInd] * sizeof(T)));
	if (bytesRead == -1) {
		util::checkReturn("reading from temp file", errno);
	}

	// move starting position
	chunkPositions[chunkInd] += chunkLen;
	buffersPos[chunkInd] = 0;
	return chunkLen;
}

bool Sorter::fillQueue(size_t chunkInd, size_t count) {
	if (chunkPositions[chunkInd] < chunkLength*(chunkInd+1) &&
			buffersPos[chunkInd] >= buffers[chunkInd].size()) {
		fillBuffer(chunkInd);
	} else if (chunkPositions[chunkInd] >= chunkLength*(chunkInd+1) &&
			   buffersPos[chunkInd] >= buffers[chunkInd].size()) {
		return false; //nope, chunk is done.
	}
	auto maxBufferPos = buffersPos[chunkInd] + count;
	for (size_t i = buffersPos[chunkInd]; i < maxBufferPos; i++) {
		auto el = buffers[chunkInd][i];
/*#ifdef DEBUG
			cout << "load " << chunkInd << ": " << el << endl;
#endif*/
		queue.emplace(pair<T,size_t>(el, chunkInd));
	}
	buffersPos[chunkInd] = maxBufferPos;
	return true;
}

} // namespace

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {
	Sorter(fdInput, size, fdOutput, memSize);
}
