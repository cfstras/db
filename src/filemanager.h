#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <cstdint>

#include "util.h"

class FileManager {
public:
	static FileManager* instance();

	// for building your own custom FileManager
	FileManager(std::string basePath);
	~FileManager();

	std::string basePath() {return basePath_;}
	int getFile(PageID pageId);

private:
	DISALLOW_COPY_AND_ASSIGN(FileManager);

	static void deconstruct(int param);
	static void deconstruct();

	static FileManager* instance_;

	std::string basePath_;
	std::mutex openFiles_mutex;
	std::unordered_map<uint16_t, int> openFiles;
};
