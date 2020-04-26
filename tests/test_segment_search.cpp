// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include <limits>
#include <utility>
#include <vector>
#include "core/segment_search.hpp"

using namespace averisera;

class SegmentSearchTest: public testing::Test
{
};

TEST_F(SegmentSearchTest,LeftInclusive)
{
	std::vector<double> v1(4);
	v1[0] = -0.3;
	v1[1] = 1.4;
	v1[2] = 4.2;
	v1[3] = std::numeric_limits<double>::infinity();
	double v2[4] = {-std::numeric_limits<double>::infinity(), 0.5, 1.5, 2.5};

    const unsigned int v1s = static_cast<unsigned int>(v1.size());
	EXPECT_EQ(-1, SegmentSearch::binary_search_left_inclusive(v1, v1s, -2.0));
	EXPECT_EQ(-1, SegmentSearch::binary_search_left_inclusive(v1, v1s, -std::numeric_limits<double>::infinity()));
	EXPECT_EQ(-1, SegmentSearch::binary_search_left_inclusive(v1, -2.0));
	EXPECT_EQ(1, SegmentSearch::binary_search_left_inclusive(v1, 2.1));
	for (size_t i = 0; i < v1.size(); ++i)
	{
		EXPECT_EQ(static_cast<int>(i), SegmentSearch::binary_search_left_inclusive(v1, v1[i]));
		EXPECT_EQ(static_cast<int>(i), SegmentSearch::binary_search_left_inclusive(v1, v1s, v1[i]));
	}	

	for (unsigned int i = 0; i < 4; ++i)
	{
		EXPECT_EQ(static_cast<int>(i), SegmentSearch::binary_search_left_inclusive(v2, 4, v2[i]));
		EXPECT_EQ(static_cast<int>(i), SegmentSearch::binary_search_left_inclusive(&v2[0], 4, v2[i]));
	}
	EXPECT_EQ(3, SegmentSearch::binary_search_left_inclusive(v2, 4, std::numeric_limits<double>::infinity()));

	std::vector<double> v3({ 0.1, 0.2, 0.3 });
	ASSERT_EQ(0, SegmentSearch::binary_search_left_inclusive(v3, 0.1));
	ASSERT_EQ(2, SegmentSearch::binary_search_left_inclusive(v3, 0.3));
}

TEST_F(SegmentSearchTest,RightInclusive)
{
	std::vector<double> v1(4);
	v1[0] = -0.3;
	v1[1] = 1.4;
	v1[2] = 4.2;
	v1[3] = std::numeric_limits<double>::infinity();
	double v2[4] = {-std::numeric_limits<double>::infinity(), 0.5, 1.5, 2.5};

    const auto v1s = static_cast<unsigned int>(v1.size());
	EXPECT_EQ(-1, SegmentSearch::binary_search_right_inclusive(v1, v1s, -2.0));
	EXPECT_EQ(-1, SegmentSearch::binary_search_right_inclusive(v1, -2.0));
	EXPECT_EQ(1, SegmentSearch::binary_search_right_inclusive(v1, 2.1));
	for (size_t i = 0; i < v1.size(); ++i)
	{
		EXPECT_EQ(i - 1, SegmentSearch::binary_search_right_inclusive(v1, v1[i]));
		EXPECT_EQ(i - 1, SegmentSearch::binary_search_right_inclusive(v1, v1s, v1[i]));
	}	

	for (size_t i = 0; i < 4; ++i)
	{
		EXPECT_EQ(i - 1, SegmentSearch::binary_search_right_inclusive(v2, 4, v2[i]));
		EXPECT_EQ(i - 1, SegmentSearch::binary_search_right_inclusive(&v2[0], 4, v2[i]));
	}
}

TEST_F(SegmentSearchTest, RightInclusiveWithGetter) {
	std::vector<std::pair<int, double>> v({ std::make_pair(1, 0.2), std::make_pair(-1, 0.3) });
	const size_t i = SegmentSearch::binary_search_right_inclusive(v, v.size(), 0.25, [](const std::vector<std::pair<int, double>>& w, size_t idx) { return w[idx].second; });
	ASSERT_EQ(0, i);
}

TEST_F(SegmentSearchTest, LeftInclusiveWithGetter) {
	std::vector<std::pair<int, double>> v({ std::make_pair(1, 0.2), std::make_pair(-1, 0.3) });
	const size_t i = SegmentSearch::binary_search_left_inclusive(v, v.size(), 0.25, [](const std::vector<std::pair<int, double>>& w, size_t idx) { return w[idx].second; });
	ASSERT_EQ(0, i);
}

TEST_F(SegmentSearchTest, ForRandomDraws) {
	std::vector<double> distr({ 0.1, 0.4, 0.9, 1.0 });
	size_t i = SegmentSearch::binary_search_right_inclusive(distr, 0.0) + 1;
	ASSERT_EQ(0, i);
	i = SegmentSearch::binary_search_right_inclusive(distr, 0.05) + 1;
	ASSERT_EQ(0, i);
	i = SegmentSearch::binary_search_right_inclusive(distr, 0.15) + 1;
	ASSERT_EQ(1, i);
	i = SegmentSearch::binary_search_right_inclusive(distr, 0.35) + 1;
	ASSERT_EQ(1, i);
	i = SegmentSearch::binary_search_right_inclusive(distr, 0.45) + 1;
	ASSERT_EQ(2, i);
	i = SegmentSearch::binary_search_right_inclusive(distr, 0.85) + 1;
	ASSERT_EQ(2, i);
	i = SegmentSearch::binary_search_right_inclusive(distr, 0.95) + 1;
	ASSERT_EQ(3, i);
	i = SegmentSearch::binary_search_right_inclusive(distr, 1.0) + 1;
	ASSERT_EQ(3, i);
	i = SegmentSearch::binary_search_right_inclusive(distr, 4u, 1.0) + 1;
	ASSERT_EQ(3, i);
}

TEST_F(SegmentSearchTest, find_index_for_padding_forward_and_backward) {
	std::vector<int> yrs({ 1990, 2001, 2005 });
	ASSERT_EQ(0, SegmentSearch::find_index_for_padding_forward_and_backward(yrs, yrs.size(), 1986));
	ASSERT_EQ(0, SegmentSearch::find_index_for_padding_forward_and_backward(yrs, yrs.size(), 1990));
	ASSERT_EQ(0, SegmentSearch::find_index_for_padding_forward_and_backward(yrs, yrs.size(), 1993));
	ASSERT_EQ(1, SegmentSearch::find_index_for_padding_forward_and_backward(yrs, yrs.size(), 2001));
	ASSERT_EQ(1, SegmentSearch::find_index_for_padding_forward_and_backward(yrs, yrs.size(), 2004));
	ASSERT_EQ(2, SegmentSearch::find_index_for_padding_forward_and_backward(yrs, yrs.size(), 2005));
	ASSERT_EQ(2, SegmentSearch::find_index_for_padding_forward_and_backward(yrs, yrs.size(), 2010));
}
