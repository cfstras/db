#pragma once

#include <cassert>

#include "util.h"

class Register {
public:

	enum class Type {
		s,
		i, u,
		l, ul,
	};

	Register(const std::string& value) :   s(value), type(Type::s){}
	Register(const     int32_t& value) :   i(value), type(Type::i){}
	Register(const    uint32_t& value) :   u(value), type(Type::u){}
	Register(const     int64_t& value) :   l(value), type(Type::l){}
	Register(const    uint64_t& value) :  ul(value), type(Type::ul){}

	~Register(){}

	void setString(std::string v) { s   = v; type = Type::s ; }
	void setInt   (    int32_t v) { i   = v; type = Type::i ; }
	void setUInt  (   uint32_t v) { u   = v; type = Type::u ; }
	void setLong  (    int64_t v) { l   = v; type = Type::l ; }
	void setULong (   uint64_t v) { ul  = v; type = Type::ul; }

	std::string getString() { assert(type == Type::s ); return s; }
	int32_t getInt()        { assert(type == Type::i ); return i; }
	uint32_t getUInt()      { assert(type == Type::u ); return u; }
	int64_t getLong()       { assert(type == Type::l ); return l; }
	uint64_t getULong()     { assert(type == Type::ul); return ul;}

	//TODO template metaprogramming would be nice here
	bool operator==(const Register &other) const {
		assert(type == other.type);
		switch (type) {
		case Type::s:
			return s == other.s;
		case Type::i:
			return i == other.i;
		case Type::u:
			return u == other.u;
		case Type::l:
			return l == other.l;
		case Type::ul:
			return ul == other.ul;
		}
	}

	bool operator!=(const Register &other) const {
		assert(type == other.type);
		switch (type) {
		case Type::s:
			return s != other.s;
		case Type::i:
			return i != other.i;
		case Type::u:
			return u != other.u;
		case Type::l:
			return l != other.l;
		case Type::ul:
			return ul != other.ul;
		}
	}

	bool operator<(const Register &other) const {
		assert(type == other.type);
		switch (type) {
		case Type::s:
			return s < other.s;
		case Type::i:
			return i < other.i;
		case Type::u:
			return u < other.u;
		case Type::l:
			return l < other.l;
		case Type::ul:
			return ul < other.ul;
		}
	}

	bool operator>(const Register &other) const {
		assert(type == other.type);
		switch (type) {
		case Type::s:
			return s > other.s;
		case Type::i:
			return i > other.i;
		case Type::u:
			return u > other.u;
		case Type::l:
			return l > other.l;
		case Type::ul:
			return ul > other.ul;
		}
	}

	bool operator<=(const Register &other) const {
		assert(type == other.type);
		switch (type) {
		case Type::s:
			return s <= other.s;
		case Type::i:
			return i <= other.i;
		case Type::u:
			return u <= other.u;
		case Type::l:
			return l <= other.l;
		case Type::ul:
			return ul <= other.ul;
		}
	}

	bool operator>=(const Register &other) const {
		assert(type == other.type);
		switch (type) {
		case Type::s:
			return s >= other.s;
		case Type::i:
			return i >= other.i;
		case Type::u:
			return u >= other.u;
		case Type::l:
			return l >= other.l;
		case Type::ul:
			return ul >= other.ul;
		}
	}

private:
	DISALLOW_COPY_AND_ASSIGN(Register);

	union{
		std::string s;
		int32_t i;
		uint32_t u;
		int64_t l;
		uint64_t ul;
	};
	Type type;
};
