#include "marshal.h"

#include <cassert>

using namespace std;

Marshal::Marshal(Schema::Relation relation) : relation(relation) {
	//TODO
}

vector<Register*> Marshal::marshal(const Record& source) {
	vector<Register*> v(relation.attributes.size());
	for (size_t i=0; i<v.size(); i++) {
		v[i] = new Register(0);
	}
	marshal(source, v);
	return v;
}

void Marshal::marshal(const Record& source, vector<Register*> registers) {
	const char* d = source.data();
	for (size_t i = 0; i<registers.size(); i++) {
		Register* reg = registers[i];
		Types::Tag type = relation.attributes[i].type;
		if (type == Types::Tag::Char) {
			size_t len = *(size_t*)(d);
			d += sizeof(size_t);
			const string s(d, len);
			d += len;
			reg->setString(s);
		} else if (type == Types::Tag::Integer) {
			reg->setLong(*(int64_t*)d);
			d += sizeof(int64_t);
		} else {
			assert(false); // Not implemented
		}
	}
}

Record Marshal::unmarshal(vector<Register*>& registers) {
	assert(registers.size() == relation.attributes.size());
	// calc total length
	size_t length = 0;
	for (size_t i = 0; i<registers.size(); i++) {
		const Register* reg = registers[i];
		const Schema::Relation::Attribute* attr = &relation.attributes[i];
		Register::Type type = reg->getType();
		Register::Type schemaType = attr->type == Types::Tag::Integer ?
				Register::Type::l : Register::Type::s;
		assert(type == schemaType);

		if (type == Register::Type::s) {
			length += sizeof(size_t);
			length += sizeof(reg->getString().size());
		} else if (type == Register::Type::l) {
			length += sizeof(int64_t);
		} else {
			assert(false); // Not implemented
		}
	}

	char data[length];
	char* d = data;
	for (size_t i=0; i<registers.size(); i++) {
		const Register* reg = registers[i];
		Register::Type type = reg->getType();
		if (type == Register::Type::s) {
			const string& s = reg->getString();
			*(size_t*)(d) = s.length();
			d += sizeof(size_t);
			memcpy(d, s.c_str(), s.length());
			d += s.length();
		} else if (type == Register::Type::l) {
			*(int64_t*)(d) = reg->getLong();
			d += sizeof(int64_t);
		} else {
			assert(false); // Not implemented
		}
	}
	return Record(length, data);
}
