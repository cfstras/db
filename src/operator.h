#pragma once

#include "util.h"
#include "register.h"

class Operator {
public:

	// Open the operator
	virtual void open() = 0;

	// Produce the next tuple
	virtual bool next() = 0;

	// Get all produced values
	virtual std::vector<Register*> getOutput() = 0;

	// Close the operator
	virtual void close() = 0;

private:
	DISALLOW_COPY_AND_ASSIGN(Operator);
};
