#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

#include "util.h"
#include "buffermanager.h"

enum class SegmentType {
	SPSegment,
	BTree,
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

class MetaSegment {
public:
	/**
	 * Loads the MetaSegment
	 */
	MetaSegment();

	/**
	 * Saves and destructs the MetaSegment
	 */
	~MetaSegment();

	/**
	 * Initializes the segment; deleting all data
	 */
	void init();

	/**
	 * Fetch a Metadata entry by name
	 */
	MetaDatum get(std::string name);

	/**
	 * Insert an entry
	 * @return false if the name is already used.
	 */
	bool put(MetaDatum datum);

private:
	DISALLOW_COPY_AND_ASSIGN(MetaSegment);

	std::unordered_map<std::string, MetaDatum> segments;
	std::vector<BufferFrame> frames;
};
