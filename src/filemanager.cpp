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

void FileManager::deconstruct(int param) { deconstruct(); }

void FileManager::deconstruct() {
	if (instance_ != nullptr) {
		delete instance_;
	}
}

FileManager* FileManager::instance_ = nullptr;

FileManager* FileManager::instance() {
	if (instance_== nullptr) {
		instance_ = new FileManager("data/");
		signal(SIGTERM|SIGSEGV|SIGINT|SIGABRT|SIGFPE, deconstruct);
		atexit(deconstruct);
	}
	return instance_;
}

FileManager::FileManager(string basePath) : basePath_(basePath) {
	if (basePath_[basePath_.length()] != '/') {
		basePath_ += '/';
	}
}

FileManager::~FileManager() {
	//TODO Tell BufferManager to close all chunks and get lost

#ifndef SILENT
	cout << "closing files." << endl;
#endif
	// close all files
	{
		lock_guard<mutex> g(openFiles_mutex);
		for (auto it = openFiles.begin(); it != openFiles.end(); it++) {
			int ret = close(it->second);
			if (ret == -1) {
				cerr << "FATAL: could not close chunk " << it->first << ": "<<
						strerror(errno) << endl;
				//TODO wait until this chunk is closed, too
			}
		}
		openFiles.clear();
	}
}

int FileManager::getFile(uint64_t pageId) {
	//TODO close some files if too many are open

	uint16_t chunk = util::chunkId(pageId);
	{
		lock_guard<mutex> g(openFiles_mutex);
		auto it = openFiles.find(chunk);
		if (it != openFiles.end()) {
			return it->second;
		}
	}

	string path = basePath() + to_string(chunk);

	// check if the folder exists
	struct stat dirStat;
	int res = stat(basePath().c_str(), &dirStat);
	if (errno == ENOENT) {
		res = mkdir(basePath().c_str(), 0700);
		if (errno != EEXIST) {
			util::checkReturn("creating chunk directory "+basePath(), res);
		}
	} else if (res == -1) {
		util::checkReturn("checking chunk directory "+basePath(), res);
	} else if (!S_ISDIR(dirStat.st_mode) && !S_ISDIR(dirStat.st_mode)) {
		// wat
		throw exception(); //TODO prettier exceptions
	}

	int fd = open(path.c_str(), O_RDWR | O_CREAT | O_SYNC, 0700);
	if (fd == -1) util::checkReturn("opening chunk file "+path, errno);
	{
		lock_guard<mutex> g(openFiles_mutex);
		openFiles.emplace(chunk, fd);
	}
	return fd;
}
