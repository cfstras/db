#include <gtest/gtest.h>

#include <fstream>

#include "testutil.h"

#include "operator.h"
#include "operator/print.h"
#include "operator/dummy.h"
#include "operator/projection.h"
#include "operator/selection.h"
#include "operator/hashjoin.h"

using namespace std;

namespace {

class OperatorTest : public ::testing::Test {
protected:

	vector<vector<string>> getData() {
		return {
			{"first", "row"},
			{"second", "row", "is", "longer"},
			{"I", "say", "wat", "wat"},
			{"next", "row", "is", "nulls"},
			{}
		};
	}

	void testOperator(Operator& op, vector<vector<string>> expect) {
		op.open();
		size_t row = 0;
		for (; row<expect.size(); row++) {
			ASSERT_EQ(true, op.next());
			auto outs = op.getOutput();
			//cerr << tupleToString(outs) << endl;
			for (size_t i=0; i<outs.size(); i++) {
				if (i < expect[row].size()) {
					EXPECT_EQ(expect[row][i], outs[i]->getString()) << "row " << row
						<< ", ind " << i;
				}
			}
		}
		EXPECT_EQ(expect.size(), row);
		ASSERT_EQ(false, op.next());
		op.close();
	}
};

TEST_F(OperatorTest, Dummy) {
	DummyOperator op(getData());
	testOperator(op, getData());
}

TEST_F(OperatorTest, Print) {
	stringstream actual;
	streambuf *coutbuf = cout.rdbuf(); // save old buf
	cout.rdbuf(actual.rdbuf()); // redirect cout

	string expected = "[first, row, , ]\n[second, row, is, longer]\n"
		+string("[I, say, wat, wat]\n[next, row, is, nulls]\n[, , , ]\n");

	shared_ptr<DummyOperator> dum(new DummyOperator(getData()));
	PrintOperator print(dum);

	print.open();
	while (print.next());
	print.close();

	EXPECT_EQ(expected, actual.str());

	cout.rdbuf(coutbuf);
}

TEST_F(OperatorTest, Projection) {
	shared_ptr<DummyOperator> dum(new DummyOperator(getData()));
	vector<size_t> cols = {2, 0};
	vector<vector<string>> expected = {
		{"", "first"},
		{"is", "second"},
		{"wat", "I"},
		{"is", "next"},
		{},
	};

	ProjectionOperator proj(dum, cols);
	testOperator(proj, expected);
}

TEST_F(OperatorTest, Selection) {
	shared_ptr<DummyOperator> dum(new DummyOperator(getData()));
	size_t row = 1;
	Register seek("row");
	vector<vector<string>> expected = {
		getData()[0],
		getData()[1],
		getData()[3],
	};

	SelectionOperator sel(dum, row, seek);
	testOperator(sel, expected);
}

} // namespace
