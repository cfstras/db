#pragma once

#include <vector>

#include "util.h"
#include "record.h"
#include "register.h"
#include "parser/schema.h"

/**
 * Marshaler takes SQL schematas and converts between Records and Registers
 */
class Marshal {
public:
	Marshal(Schema schema);

	std::vector<Register> marshal(const Record& source);
	void marshal(const Record& source, std::vector<Register> registers);

	Record unmarshal(std::vector<Register>& registers);

private:
	DISALLOW_COPY_AND_ASSIGN(Marshal);

	Schema schema;
};
