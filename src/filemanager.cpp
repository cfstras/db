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

FileManager* FileManager::instance_ = nullptr;
shared_ptr<FileManager> alwaysInstance = nullptr;

shared_ptr<FileManager> FileManager::instance() {
	if (instance_== nullptr) {
		instance_ = new FileManager("data/");
		alwaysInstance.reset(instance_);
	}
	return alwaysInstance;
}

FileManager::FileManager(string basePath) : basePath_(basePath) {
	if (basePath_[basePath_.length()] != '/') {
		basePath_ += '/';
	}
}

FileManager::~FileManager() {
	//TODO Tell BufferManager to close all segments and get lost

#ifndef SILENT
	cout << "closing files." << endl;
#endif
	// close all files
	{
		lock_guard<mutex> g(openFiles_mutex);
		for (auto it = openFiles.begin(); it != openFiles.end(); it++) {
			int ret = close(it->second);
			if (ret == -1) {
				cerr << "FATAL: could not close segment " << it->first << ": "<<
						strerror(errno) << endl;
				//TODO wait until this segment is closed, too
			}
		}
		openFiles.clear();
	}
}

int FileManager::getFile(PageID pageId) {
	//TODO close some files if too many are open

	uint16_t segment = util::extractSegmentFromPageID(pageId);
	{
		lock_guard<mutex> g(openFiles_mutex);
		auto it = openFiles.find(segment);
		if (it != openFiles.end()) {
			return it->second;
		}
	}

	string path = basePath() + to_string(segment);

	// check if the folder exists
	struct stat dirStat;
	int res = stat(basePath().c_str(), &dirStat);
	if (errno == ENOENT) {
		res = mkdir(basePath().c_str(), 0700);
		if (errno != EEXIST) {
			util::checkReturn("creating segment directory "+basePath(), res);
		}
	} else if (res == -1) {
		util::checkReturn("checking segment directory "+basePath(), res);
	} else if (!S_ISDIR(dirStat.st_mode) && !S_ISDIR(dirStat.st_mode)) {
		// wat
		throw exception(); //TODO prettier exceptions
	}

	int fd = open(path.c_str(), O_RDWR | O_CREAT | O_SYNC, 0700);
	if (fd == -1) util::checkReturn("opening segment file "+path, errno);
	{
		lock_guard<mutex> g(openFiles_mutex);
		openFiles.emplace(segment, fd);
	}
	return fd;
}
