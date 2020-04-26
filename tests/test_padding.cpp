// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/padding.hpp"
#include <array>
#include <vector>

using namespace averisera;

TEST(Padding, get_padded) {
	std::array<double, 2> values = { 0.3, 0.1 };
	std::vector<int> indices = { 4, 10 };
	ASSERT_EQ(0.3, Padding::get_padded(indices, values, 2, 2));
	ASSERT_EQ(0.1, Padding::get_padded(indices, values, 2, 12));
	std::vector<double> values2 = { 0.25, 0.45 };
	ASSERT_EQ(0.25, Padding::get_padded(indices, values2, 5));	
}

TEST(Padding, pad) {
	std::array<double, 2> values = { 0.3, 0.1 };
	std::vector<int> indices = { 4, 6 };	
	std::vector<double> new_values(5);
	Padding::pad(indices, values, 3, 8, new_values);
	ASSERT_EQ(std::vector<double>({ 0.3, 0.3, 0.3, 0.1, 0.1 }), new_values);
}

TEST(Padding, pad_nan) {
	double nan = std::numeric_limits<double>::quiet_NaN();
	std::vector<double> values({ nan, 0.1, nan, 0.3, 0.4, nan });
	ASSERT_TRUE(Padding::pad_nan(values));
	ASSERT_EQ(std::vector<double>({ 0.1, 0.1, 0.1, 0.3, 0.4, 0.4 }), values);
	values = { nan, nan };
	ASSERT_FALSE(Padding::pad_nan(values));
	values = { 0.1, nan, 0.3 };
	ASSERT_TRUE(Padding::pad_nan(values));
	ASSERT_EQ(std::vector<double>({ 0.1, 0.1, 0.3 }), values);
}

TEST(Padding, pad_unknown) {
	double nan = 0.0;
	std::vector<double> values = { nan, 0.1, nan, 0.3, 0.4, nan };
	ASSERT_TRUE(Padding::pad_unknown(values, nan));
	ASSERT_EQ(std::vector<double>({ 0.1, 0.1, 0.1, 0.3, 0.4, 0.4 }), values);
	values = { nan, nan };
	ASSERT_FALSE(Padding::pad_unknown(values, nan));
	values = { 0.1, nan, 0.3 };
	ASSERT_TRUE(Padding::pad_unknown(values, nan));
	ASSERT_EQ(std::vector<double>({ 0.1, 0.1, 0.3 }), values);
}

TEST(Padding, pad_nan_rows) {
	const double nan = std::numeric_limits<double>::quiet_NaN();
	Eigen::MatrixXd m(2, 3);
	m << nan, nan, nan,
		0.1, nan, 0.3;
	ASSERT_FALSE(Padding::pad_nan_rows(m));
	ASSERT_EQ(0.1, m(1, 1));
	ASSERT_TRUE(std::isnan(m(0, 1)));
}

TEST(Padding, pad_nan_cols) {
	const double nan = std::numeric_limits<double>::quiet_NaN();
	Eigen::MatrixXd m(2, 3);
	m << nan, nan, 0.3,
		0.1, nan, nan;
	ASSERT_FALSE(Padding::pad_nan_cols(m));
	ASSERT_EQ(0.1, m(0, 0));
	ASSERT_EQ(0.3, m(1, 2));
	ASSERT_TRUE(std::isnan(m(1, 1)));
}
