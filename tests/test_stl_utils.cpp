// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/dates.hpp"
#include "core/stl_utils.hpp"

using namespace averisera;

TEST(StlUtils, SetUnion) {
    std::set<int> s1({1, 2, 3});
    std::set<int> s2({4, 3});
    const std::set<int> s3 = StlUtils::set_union(s1, s2);
    ASSERT_EQ(4u, s3.size());
    for (int i = 1; i <= 4; ++i) {
        ASSERT_EQ(1u, s3.count(i)) << i;
    }
}

TEST(StlUtils, UnorderedSetUnion) {
    std::unordered_set<int> s1({ 1, 2, 3 });
    std::unordered_set<int> s2({ 4, 3 });
    const std::unordered_set<int> s3 = StlUtils::set_union(s1, s2);
    ASSERT_EQ(4u, s3.size());
    for (int i = 1; i <= 4; ++i) {
        ASSERT_EQ(1u, s3.count(i)) << i;
    }
}

TEST(StlUtils, MapGet) {
	std::map<std::string, double> map;
	map["FOO"] = 0.345;
	ASSERT_EQ(0.345, StlUtils::get(map, std::string("FOO"), 0.12));
	ASSERT_EQ(0.12, StlUtils::get(map, std::string("BAR"), 0.12));
}

TEST(StlUtils, UnorderedMapGet) {
	std::unordered_map<std::string, double> map;
	map["FOO"] = 0.345;
	ASSERT_EQ(0.345, StlUtils::get(map, std::string("FOO"), 0.12));
	ASSERT_EQ(0.12, StlUtils::get(map, std::string("BAR"), 0.12));
}

TEST(StlUtils, PrintPair) {
    std::stringstream ss;
    ss << std::make_pair(Date(2013, 4, 23), -0.23);
    ASSERT_EQ("<2013-04-23, -0.23>", ss.str());
}

TEST(StlUtils, PrintVectorDouble) {
    std::vector<double> v({0.2, 0.21});
    std::stringstream ss;
    ss << v;
    ASSERT_EQ("[0.2, 0.21]", ss.str());
}

TEST(StlUtils, PrintVectorUInt8) {
	std::vector<uint8_t> v({ 0, 20 });
	std::stringstream ss;
	ss << v;
	ASSERT_EQ("[0, 20]", ss.str());
}

TEST(StlUtils, PrintVectorInt8) {
	std::vector<int8_t> v({ 0, -2, 20 });
	std::stringstream ss;
	ss << v;
	ASSERT_EQ("[0, -2, 20]", ss.str());
}

TEST(StlUtils, PrintVectorDates) {
    std::vector<Date> v({Date(1939, 9, 1), Date(1989, 6, 4)});
    std::stringstream ss;
    ss << v;
    ASSERT_EQ("[1939-09-01, 1989-06-04]", ss.str());
}

TEST(StlUtils, MergeSortedVectors) {
	const std::vector<int> years1({ 1990, 1991, 1994, 1995 });
	const std::vector<int> years2({ 1989, 1990, 1993, 1994 });
	const std::vector<int> years(StlUtils::merge_sorted_vectors(years1, years2));
	ASSERT_EQ(std::vector<int>({ 1989, 1990, 1991, 1993, 1994, 1995 }), years);	
}
