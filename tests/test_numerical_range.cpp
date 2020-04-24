#include <gtest/gtest.h>
#include "core/numerical_range.hpp"
#include <boost/lexical_cast.hpp>

using namespace averisera;


TEST(NumericalRange, IsAdjacentTo) {
	const NumericalRange<double> r1(0.1, 0.2);
	const NumericalRange<double> r2(0.2, 0.3);
	const NumericalRange<double> r3(0.1, 0.3);
	ASSERT_TRUE(r1.is_adjacent_to(r2));
	ASSERT_TRUE(r2.is_adjacent_to(r1));
	ASSERT_FALSE(r1.is_adjacent_to(r3));
	ASSERT_FALSE(r1.is_adjacent_to(r1));
}

TEST(NumericalRange, IsDisjointWith) {
	const NumericalRange<double> r1(0.1, 0.2);
	const NumericalRange<double> r2(0.2, 0.3);
	const NumericalRange<double> r3(0.15, 0.25);
	ASSERT_TRUE(r1.is_disjoint_with(r2));
	ASSERT_TRUE(r2.is_disjoint_with(r1));
	ASSERT_FALSE(r1.is_disjoint_with(r3));
	ASSERT_FALSE(r2.is_disjoint_with(r3));
	ASSERT_FALSE(r1.is_disjoint_with(r1));
}

TEST(NumericalRange, AllDisjoint) {
	std::vector<NumericalRange<int>> v({ NumericalRange<int>(0, 2), NumericalRange<int>(4, 6), NumericalRange<int>(2, 4) });
	ASSERT_TRUE(NumericalRange<int>::all_disjoint(v));
	v[2] = NumericalRange<int>(2, 10);
	ASSERT_FALSE(NumericalRange<int>::all_disjoint(v));
	std::sort(v.begin(), v.end());
	ASSERT_FALSE(NumericalRange<int>::all_disjoint(v, true));
	v[1] = NumericalRange<int>(2, 3);
	ASSERT_TRUE(NumericalRange<int>::all_disjoint(v, true));
	v.clear();
	ASSERT_TRUE(NumericalRange<int>::all_disjoint(v, false));
	ASSERT_TRUE(NumericalRange<int>::all_disjoint(v, true));
}

TEST(NumericalRange, AllAdjacent) {
	std::vector<NumericalRange<int>> v({ NumericalRange<int>(0, 2), NumericalRange<int>(2, 4), NumericalRange<int>(4, 6) });
	ASSERT_TRUE(NumericalRange<int>::all_adjacent(v));
	v[0] = NumericalRange<int>(0, 1);
	ASSERT_FALSE(NumericalRange<int>::all_adjacent(v));
	v[0] = NumericalRange<int>(0, 2);
	v[1] = NumericalRange<int>(2, 5);
	ASSERT_FALSE(NumericalRange<int>::all_adjacent(v));
}

TEST(NumericalRange, FromStringOpenEnded) {
	ASSERT_EQ(NumericalRange<int>(1, 3), NumericalRange<int>::from_string_open_ended("1-2", [](const std::string& str) { return boost::lexical_cast<int>(str); }, nullptr, nullptr));
	int i = 4;
	ASSERT_EQ(NumericalRange<int>(1, 5), NumericalRange<int>::from_string_open_ended("1-", [](const std::string& str) { return boost::lexical_cast<int>(str); }, nullptr, &i));
	ASSERT_EQ(NumericalRange<int>(4, 11), NumericalRange<int>::from_string_open_ended("-10", [](const std::string& str) { return boost::lexical_cast<int>(str); }, &i, nullptr));
	int j = 10;
	ASSERT_EQ(NumericalRange<int>(4, 11), NumericalRange<int>::from_string_open_ended("-", [](const std::string& str) { return boost::lexical_cast<int>(str); }, &i, &j));
	ASSERT_EQ(NumericalRange<int>(2, 3), NumericalRange<int>::from_string_open_ended("2", [](const std::string& str) { return boost::lexical_cast<int>(str); }, nullptr, nullptr));
}

TEST(NumericalRange, AddDelta) {
	const NumericalRange<double> r1(0.1, 0.2);
	const NumericalRange<double> r2 = r1 + 1.0;
	ASSERT_NEAR(1.1, r2.begin(), 1E-15);
	ASSERT_NEAR(1.2, r2.end(), 1E-15);
}
