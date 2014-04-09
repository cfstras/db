#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "sort.h"
#include "util.h"

using namespace std;

int testSort();

int main() {
	cout << "Hello, World!" << endl;

	return testSort();
}

int testSort() {
	int in = open("small.bin", O_RDONLY);
	int out = open("sort.bin", O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (in == -1) {
		util::checkReturn("opening input", errno);
	}
	if (out == -1) {
		util::checkReturn("opening output", errno);
	}
	externalSort(in, 40, out, 10 * 8);

	util::checkReturn("closing input", close(in));
	util::checkReturn("closing output", close(out));
	return 0;
}
