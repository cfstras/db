#pragma once

#include "util.h"

// A simple Record implementation
class Record {
public:
	// Move Constructor
	Record(Record&& t);
	// Constructor
	Record(unsigned len, const char* const ptr);
	// Destructor
	~Record();

	// Get pointer to data
	const char* data() const;
	// Get data size in bytes
	unsigned len() const;

private:
	DISALLOW_COPY_AND_ASSIGN(Record);

	unsigned len_;
	char* data_;
};
