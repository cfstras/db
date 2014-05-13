#include "record.h"

#include <cstdlib>
#include <cstring>
#include <cassert>

using namespace std;

Record::Record(Record&& t) : len_(t.len_), data_(t.data_) {
	t.data_ = nullptr;
	t.len_ = 0;
}

Record::Record(unsigned len, const char* const ptr) : len_(len) {
	data_ = new char[len_];
	if (len != 0) {
		assert(ptr != nullptr);
		memcpy(data_, ptr, len_);
	}
}

const char* Record::data() const {
	return data_;
}

unsigned Record::len() const {
	return len_;
}

Record::~Record() {
	delete[] data_;
}
