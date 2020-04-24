/*
(C) Averisera Ltd 2014
*/
#include <gtest/gtest.h>
#include "core/vectors.hpp"

using namespace averisera::Vectors;

TEST(Vectors, Ensure) {
	std::vector<double> v;
	ensure_elem(0, v);
	ASSERT_EQ(1u, v.size());
	ensure_range(1, 3, v);
	ASSERT_EQ(3u, v.size());
}

