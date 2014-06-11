#pragma once

#include "util.h"

class Register {
public:

	void setString(std::string v) { value.s = v; }
	void setInt(int32_t v) { value.i = v; }
	void setUInt(uint32_t v) { value.ui = v; }
	void setLong(int64_t v) { value.li = v; }
	void setULong(uint64_t v) { value.lui = v; }

	std::string getString() { return value.s; }
	int32_t getInt() { return value.i; }
	uint32_t getUInt() { return value.ui; }
	int64_t getLong() { return value.li; }
	uint64_t getULong() { return value.lui; }

private:
	DISALLOW_COPY_AND_ASSIGN(Register);

	union {
		std::string s;
		int32_t i;
		uint32_t ui;
		int64_t li;
		uint64_t lui;
	} value;
};
