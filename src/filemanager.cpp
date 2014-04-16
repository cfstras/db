#include "filemanager.h"

#include <iostream>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <cstdlib>

#include "util.h"

using namespace std;

void FileManager::deconstruct(int param) {
	if (instance_ != nullptr) {
		delete instance_;
	}
}

FileManager* FileManager::instance() {
	if (instance_== nullptr) {
		instance_ = new FileManager("data/");
		signal(SIGTERM|SIGSEGV|SIGINT|SIGABRT|SIGFPE, deconstruct);
		atexit(deconstruct);
	}
	return instance_;
}

FileManager::FileManager(string basePath) : basePath_(basePath) {
}

FileManager::~FileManager() {
	//TODO Tell BufferManager to close all chunks and get lost

	// close all files
	for (auto it = openFiles.begin(); it != openFiles.end(); it++) {
		int ret = close(it->second);
		if (ret == -1) {
			cerr << "FATAL: could not close chunk " << it->first << ": "<<
					strerr(errno) << endl;
		} else {
			openFiles.erase(it);
		}
	}
}

int FileManager::chunkId(uint64_t pageId) {
	return (pageId & 0xffff000000000000LL) << 6;
}

int FileManager::getFile(uint64_t pageId) {
	//TODO close some files if too many are open

	uint16_t chunkId = chunkId(pageId);
	auto it = openFiles.find(chunkId);
	if (it != openFiles.end()) {
		return it->second;
	}

	string path = basePath() + to_string(chunkId);
	int fd = open(path.c_str(), O_RDWR | O_CREAT | O_DIRECT | O_SYNC);
	if (fd == -1) util::checkReturn("opening chunk file "+path, errno);
	openFiles.emplace(chunkId, fd);
	return fd;
}
