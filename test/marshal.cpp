#include <gtest/gtest.h>

#include <algorithm>

#include "marshal.h"
#include "parser/parser.h"

using namespace std;

namespace {

class MarshalTest : public ::testing::Test {
protected:
	void SetUp() {

	}

	Schema::Relation sch(string v) {
		stringstream in(v);
		Parser p(&in);
		return p.parse()->relations[0];
	}

	void eq(vector<Register*> a, vector<Register*> b) {
		ASSERT_EQ(a.size(), b.size());
		for (size_t i = 0; i<a.size(); i++) {
			EXPECT_EQ(*a[i], *b[i])
				<<"Expected "<<a[i]->toString()<<", got "<<b[i]->toString();
		}
	}
};

TEST_F(MarshalTest, String) {
	auto s = sch("create table bla (name char(3));");
	Marshal m(s);
	Register r("abc");
	vector<Register*> v {&r};

	Record rec = m.unmarshal(v);
#ifndef SILENT
	cerr << "Unmarshaled: " << string(rec.data(), rec.len()) << endl;
#endif
	auto v2 = m.marshal(rec);
	eq(v, v2);
}

TEST_F(MarshalTest, IntString) {
	auto s = sch("create table bla (int integer, name char(3));");
	Marshal m(s);
	Register r1(123L), r2("abc");
	vector<Register*> v {&r1, &r2};

	Record rec = m.unmarshal(v);
#ifndef SILENT
	cerr << "Unmarshaled: " << string(rec.data(), rec.len()) << endl;
#endif
	auto v2 = m.marshal(rec);
	eq(v, v2);
}

} // namespace
