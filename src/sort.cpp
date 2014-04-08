#include "sort.h"

#include <unistd.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <queue>

using namespace std;

typedef uint64_t T;

void externalSort(int fdInput, uint64_t size, int fdOutput, uint64_t memSize) {

	//TODO the length should be less, since our sorting algorithm and prio-queue
	// take up space, too.
	size_t length = memSize / sizeof(T);
	vector<T> chunk(length);

	priority_queue<T> queue;
	ssize_t bytesRead;
	uint64_t elementsSorted;
	while ((bytesRead = read(fdInput, &chunk[0], length*sizeof(T))) > 0) {
		ssize_t elementsRead = bytesRead / static_cast<ssize_t>(sizeof(T));

		auto begin = chunk.begin(), end = chunk.begin() + elementsRead;

		sort(begin, end);

		for (auto i = begin; i != end; i++) {
			cout << *i << endl;
		}
		cout << endl;

		//TODO
	}

}
