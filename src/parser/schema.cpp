#include "schema.h"

#include <sstream>

using namespace std;

static string type(const Schema::Relation::Attribute& attr) {
	Types::Tag type = attr.type;
	switch(type) {
		case Types::Tag::Integer:
			return "Integer";
		/*case Types::Tag::Numeric: {
			stringstream ss;
			ss << "Numeric(" << attr.len1 << ", " << attr.len2 << ")";
			return ss.str();
		}*/
		case Types::Tag::Char: {
			stringstream ss;
			ss << "Char(" << attr.len << ")";
			return ss.str();
		}
	}
	throw;
}

string Schema::toString() const {
	stringstream out;
	for (const Schema::Relation& rel : relations) {
		out << "create table " << rel.name << " (" << endl;

		for (const auto& attr : rel.attributes) {
			out << '\t' << attr.name << '\t' << type(attr)
					<< (attr.notNull ? "\tnot null" : "")
					<< "," << endl;
		}

		out << "\tprimary key (";
		bool first=true;
		for (unsigned keyId : rel.primaryKey) {
			if (first) {
				first = false;
			} else {
				out << ", ";
			}
			out << rel.attributes[keyId].name;
		}
		out << ')' << endl;
		out << ");" << endl;
	}
	return out.str();
}
