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

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {

	//TODO the length should be less, since our sorting algorithm and prio-queue
	// take up space, too.
	size_t chunkLength = memSize / sizeof(T);
	vector<T> chunk(chunkLength);

	// create temp file
	char tempTemplate[] ="db-externalsortXXXXXX";
	int fdTemp = mkstemp(tempTemplate);
	if (fdTemp == -1) {
		util::checkReturn("creating chunk tempfile", errno);
	}

	size_t tempFileSize = size * sizeof(T);
	util::checkReturn("allocating chunk tempfile",
		posix_fallocate(fdTemp, 0, static_cast<off_t>(tempFileSize)));

	// save the positions (as index) of the chunk starting points
	vector<uint64_t> chunkPositions;
	chunkPositions.reserve(size * sizeof(T) / memSize);

	// sort chunks
	ssize_t bytesRead;
	uint64_t elementsSorted = 0;
	size_t maxChunkBytes = chunkLength*sizeof(T);
	while ((bytesRead = read(fdInput, &chunk[0], maxChunkBytes)) > 0 &&
			elementsSorted < size) {

		uint64_t elementsRead = static_cast<uint64_t>(bytesRead) / sizeof(T);

		// redefine the maxChunkBytes
		maxChunkBytes = min(maxChunkBytes, static_cast<size_t>(bytesRead));

		auto begin = chunk.begin();
		auto end = chunk.begin() + static_cast<int64_t>(elementsRead);
		sort(begin, end);

		// write chunk to temp file
		auto written = write(fdTemp, &chunk[0], maxChunkBytes);
		if (written < 0) {
			util::checkReturn("writing chunk to tempfile", errno);
		}

		chunkPositions.push_back(elementsSorted);
		elementsSorted += elementsRead;
	}

	// now merge.
	//priority_queue<T> queue;

	// get some space in the output file
	off_t offset = lseek(fdOutput, 0, SEEK_CUR);
	util::checkReturn("allocating output space",
		posix_fallocate(fdOutput, offset, static_cast<off_t>(size * sizeof(T))));

	// buffers.
	// one space left for output buffer
	auto bufferSize = memSize / ( 1 + chunkPositions.size());
	vector<vector<T>> buffers(bufferSize / sizeof(T));
	for (vector<T> &v : buffers) {
		v.reserve(bufferSize);
	}

	elementsSorted = 0;
	while (elementsSorted < size) {
		// read start of chunks
		// put in prio queue
		// pull from queue and write out
		//TODO
		elementsSorted = size;
	}

	/*for (auto i = begin; i != end; i++) {
		cout << *i << endl;
	}
	cout << endl; */

}
