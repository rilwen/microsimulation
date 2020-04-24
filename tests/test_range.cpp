#include <gtest/gtest.h>
#include "core/range.hpp"
#include <boost/lexical_cast.hpp>

using namespace averisera;

TEST(Range, Test) {
	const Range<int> r(1, 2);
	ASSERT_EQ(1, r.begin());
	ASSERT_EQ(2, r.end());
	ASSERT_THROW(Range<int>(2, 1), std::domain_error);
	const Range<int> r2(1, 3);
	ASSERT_EQ(r, r);
	ASSERT_NE(r, r2);
	const Range<int> r3(0, 2);
	ASSERT_NE(r3, r);
	ASSERT_LT(r3, r);
}

TEST(Range, FromString) {
	ASSERT_EQ(Range<int>(1, 2), Range<int>::from_string("1-2", [](const std::string& str) { return boost::lexical_cast<int>(str); }, nullptr, nullptr));
	int i = 4;
	ASSERT_EQ(Range<int>(1, 4), Range<int>::from_string("1-", [](const std::string& str) { return boost::lexical_cast<int>(str); }, nullptr, &i));
	ASSERT_EQ(Range<int>(4, 10), Range<int>::from_string("-10", [](const std::string& str) { return boost::lexical_cast<int>(str); }, &i, nullptr));
	int j = 10;
	ASSERT_EQ(Range<int>(4, 10), Range<int>::from_string("-", [](const std::string& str) { return boost::lexical_cast<int>(str); }, &i, &j));
	ASSERT_EQ(Range<int>(2, 2), Range<int>::from_string("2", [](const std::string& str) { return boost::lexical_cast<int>(str); }, nullptr, nullptr));
}

enum class Color {
	RED,
	GREEN,
	BLUE,
	YELLOW,
	MAGENTA
};

TEST(Range, IsDisjointWith) {
	const Range<Color> r1(Color::RED, Color::BLUE);
	const Range<Color> r2(Color::YELLOW, Color::MAGENTA);
	const Range<Color> r3(Color::BLUE, Color::YELLOW);
	ASSERT_TRUE(r1.is_disjoint_with(r2));
	ASSERT_TRUE(r2.is_disjoint_with(r1));
	ASSERT_FALSE(r1.is_disjoint_with(r3));
	ASSERT_FALSE(r1.is_disjoint_with(r1));
	ASSERT_FALSE(r3.is_disjoint_with(r1));
}

TEST(Range, AllDisjoint) {
	std::vector<Range<int>> v({ Range<int>(0, 1), Range<int>(4, 5), Range<int>(2, 3) });
	ASSERT_TRUE(Range<int>::all_disjoint(v));
	v[2] = Range<int>(2, 10);
	ASSERT_FALSE(Range<int>::all_disjoint(v));
	std::sort(v.begin(), v.end());
	ASSERT_FALSE(Range<int>::all_disjoint(v, true));
	v[1] = Range<int>(2, 2);
	ASSERT_TRUE(Range<int>::all_disjoint(v, true));
	v.clear();
	ASSERT_TRUE(Range<int>::all_disjoint(v, false));
	ASSERT_TRUE(Range<int>::all_disjoint(v, true));
}

TEST(Range, Contains) {
	const Range<Color> r1(Color::RED, Color::BLUE);
	const Range<Color> r2(Color::YELLOW, Color::MAGENTA);
	const Range<Color> r3(Color::BLUE, Color::YELLOW);
	ASSERT_TRUE(r1.contains(r1));
	ASSERT_FALSE(r1.contains(r2));
	ASSERT_FALSE(r1.contains(r3));
	const Range<Color> r4(Color::RED, Color::GREEN);
	const Range<Color> r5(Color::GREEN, Color::BLUE);
	ASSERT_TRUE(r1.contains(r4));
	ASSERT_TRUE(r1.contains(r5));
	ASSERT_FALSE(r4.contains(r1));
}

TEST(Range, Inclusion) {
	ASSERT_EQ(InclusionRelation::CONTAINS, Range<int>(0, 9).inclusion(Range<int>(2, 6)));
	ASSERT_EQ(InclusionRelation::IS_CONTAINED_BY, Range<int>(4, 5).inclusion(Range<int>(2, 6)));
	ASSERT_EQ(InclusionRelation::EQUALS, Range<int>(2, 6).inclusion(Range<int>(2, 6)));
	ASSERT_EQ(InclusionRelation::UNDEFINED, Range<int>(0, 5).inclusion(Range<int>(3, 7)));
}

TEST(Range, IsContainedByAny) {
	std::vector<Range<int>> v({ Range<int>(0, 1), Range<int>(2, 3), Range<int>(2, 5) });
	Range<int> r(2, 4);
	ASSERT_TRUE(r.is_contained_by_any(v));
	r = Range<int>(1, 7);
	ASSERT_FALSE(r.is_contained_by_any(v));
}

