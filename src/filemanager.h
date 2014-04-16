#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

#include "util.h"

class FileManager {
public:
	static FileManager* instance();

	std::string basePath() {return basePath_;}
	uint16_t chunkId(uint64_t pageId);
	int getFile(uint64_t pageId);

private:
	DISALLOW_COPY_AND_ASSIGN(FileManager);
	FileManager(std::string basePath);
	~FileManager();

	static void deconstruct(int param);

	static FileManager* instance_;

	std::string basePath_;
	std::unordered_map<uint16_t, int> openFiles;
};
