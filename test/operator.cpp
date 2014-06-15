#include <gtest/gtest.h>

#include <fstream>

#include "testutil.h"

#include "operator.h"
#include "operator/print.h"
#include "operator/dummy.h"

using namespace std;

namespace {

TEST(DummyOperator, Test) {
	vector<vector<string>> src = {
		{"first row", "yay"},
		{"second", "row", "is", "longer"},
		{"I", "say", "wat", "wat"},
		{"next row is nulls"},
		{}
	};

	vector<vector<string>> srccopy(src);
	DummyOperator op(srccopy);
	op.open();
	size_t row = 0;
	for (; row<src.size(); row++) {
		ASSERT_EQ(true, op.next());
		auto outs = op.getOutput();
		//cerr << tupleToString(outs) << endl;
		for (size_t i=0; i<outs.size(); i++) {
			if (i < src[row].size()) {
				EXPECT_EQ(src[row][i], outs[i]->getString()) << "row " << row
					<< ", ind " << i;
			}
		}
	}
	EXPECT_EQ(src.size(), row);
	ASSERT_EQ(false, op.next());
}

TEST(PrintOperator, Test) {
	stringstream actual;
	streambuf *coutbuf = cout.rdbuf(); // save old buf
	cout.rdbuf(actual.rdbuf()); // redirect cout

	vector<vector<string>> src = {
		{"first row", "yay"},
		{"second", "row", "is", "longer"},
	};
	string expected = "[first row, yay, , ]\n[second, row, is, longer]\n";

	shared_ptr<DummyOperator> dum(new DummyOperator(src));
	PrintOperator print(dum);

	print.open();
	while (print.next());
	print.close();

	EXPECT_EQ(expected, actual.str());

	cout.rdbuf(coutbuf);
}

} // namespace
