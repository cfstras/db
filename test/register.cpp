#include <gtest/gtest.h>

#include "register.h"

using namespace std;

namespace {

TEST(RegisterTest, Cons) {
	Register r1("asdf");
	Register r2(2);
}

/**
 * a == b
 * a < c
 */
template<typename T>
void testEQ(T& a, T& b, T& c) {
	EXPECT_EQ(a, b);
	EXPECT_NE(a, c);

	EXPECT_LT(a, c);
	EXPECT_LE(a, c);
	EXPECT_LE(a, a);

	EXPECT_GT(c, a);
	EXPECT_GE(c, a);
	EXPECT_GE(a, a);
}

TEST(RegisterTest, Ops) {
	{
		Register r1("asdf"), r2("asdf"), r3("bsdf");
		testEQ(r1, r2, r3);
	}
	{
		Register a(1), b(1), c(2);
		testEQ(a, b, c);
	}
}

} // namespace
