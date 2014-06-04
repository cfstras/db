#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <memory>

#include "util.h"
#include "buffermanager.h"

enum class SegmentType {
	SPSegment,
	BTree,

	TYPE_COUNT
};

#pragma pack(push)
#pragma pack(1)
struct MetaDatum {
	std::string name;
	SegmentID segmentID;
	PageID numPages;
	SegmentType type;

	// for relations: use schemaparser
	// for indices: btrees can load this
	std::string payload;
};
#pragma pack(pop)

class MetaSegment {
public:
	/**
	 * Loads the MetaSegment
	 */
	MetaSegment(std::shared_ptr<BufferManager> bm);

	/**
	 * Loads or creates a new MetaSegment
	 */
	MetaSegment(std::shared_ptr<BufferManager> bm, bool init);

	/**
	 * Saves and destructs the MetaSegment
	 */
	~MetaSegment();

	/**
	 * Fetch a Metadata entry by name
	 */
	MetaDatum get(std::string name);

	/**
	 * Insert an entry
	 * @return false if the name is already used.
	 */
	bool put(const MetaDatum &datum);

private:
	DISALLOW_COPY_AND_ASSIGN(MetaSegment);

	std::shared_ptr<BufferManager> bufferManager;

	std::unordered_map<std::string, MetaDatum> segments;
	std::vector<BufferFrame*> frames;
};
