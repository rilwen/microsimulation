#include <gtest/gtest.h>
#include <vector>
#include "core/index_shifter.hpp"

using namespace averisera;

TEST(IndexShifterTest,Test)
{
	std::vector<double> vec(3);
	vec[0] = -1;
	vec[1] = 2;
	vec[2] = 0.5;
	IndexShifter<double> sv(-1);
	EXPECT_EQ(vec[0], sv.idx(vec,-1));
	EXPECT_EQ(vec[1], sv.idx(vec,0));
	EXPECT_EQ(vec[2], sv.idx(vec,1));

	ShiftedVector<std::vector<double>,double> sv2(vec, -1);
	EXPECT_EQ(vec[0], sv2[-1]);
	EXPECT_EQ(vec[1], sv2[0]);
	EXPECT_EQ(vec[2], sv2[1]);

	sv2[-1] = 5;
	EXPECT_EQ(5, sv2[-1]);
	EXPECT_NE(vec[0], sv2[-1]);
}