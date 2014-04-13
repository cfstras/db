#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "sort.h"
#include "util.h"

using namespace std;

int doSort(char* inFile, char* outFile, uint64_t bufferMB);

void help();

int main(int argc, char** argv) {
	if (argc != 4) {
		help();
		return 1;
	}
	int64_t buf = atoll(argv[3]);
	if (buf <= 0) { help(); return 1; }
	return doSort(argv[1], argv[2], static_cast<uint64_t>(buf));
}

void help() {
	cout << "Aufruf:" << endl <<
			"\tsort <inputFile> <outputFile> <memoryBufferInMB>" << endl;
}

int doSort(char* inFile, char* outFile, uint64_t bufferMB) {
	int in = open(inFile, O_RDONLY);
	int out = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0700);
	if (in == -1) {
		util::checkReturn("opening input", errno);
	}
	if (out == -1) {
		util::checkReturn("opening output", errno);
	}
	externalSort(in, UINT64_MAX, out, bufferMB * 1024 * 1024);

	util::checkReturn("closing input", close(in));
	util::checkReturn("closing output", close(out));
	return 0;
}
