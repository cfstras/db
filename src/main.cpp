#include <iostream>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>

#include "sort.h"

using namespace std;

int testSort();
void checkReturn(string what, int err);

int main() {
	cout << "Hello, World!" << endl;
	return testSort();
}

void checkReturn(string what, int err) {
	if (err != 0) {
		throw runtime_error("Error while " + what + ": " + strerror(err));
	}
}

int testSort() {
	int in = open("small.bin", O_RDONLY);
	int out = open("sort.bin", O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (in == -1) {
		checkReturn("opening input", errno);
	}
	if (out == -1) {
		checkReturn("opening output", errno);
	}
	externalSort(in, 40, out, 10 * 8);

	checkReturn("closing input", close(in));
	checkReturn("closing output", close(out));
	return 0;
}
