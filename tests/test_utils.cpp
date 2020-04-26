// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/dates.hpp"
#include "core/utils.hpp"
#include <vector>
#include <cmath>
#include <limits>
#include <map>
#include <sstream>
#include <unordered_map>
#include <utility>
#include "testing/assertions.hpp"


using namespace averisera;


TEST(Utils, RangeIdx) {
	std::vector<double> bnds = { 0, 1, 2, 3 };
	ASSERT_EQ(0u, Utils::range_idx(bnds, -1.0));
	ASSERT_EQ(0u, Utils::range_idx(bnds, 0.));
	ASSERT_EQ(1u, Utils::range_idx(bnds, 0.5));
	ASSERT_EQ(3u, Utils::range_idx(bnds, 3.));
	ASSERT_EQ(4u, Utils::range_idx(bnds, 3.1));
}



TEST(Utils, PassThrough) {
    ASSERT_EQ(2, Utils::pass_through(2, [](){}));
    double a = 1;
    ASSERT_EQ(2, Utils::pass_through(a, [&a](){++a;}));
    ASSERT_THROW(Utils::pass_through(2, [](){ throw std::runtime_error(""); }), std::runtime_error);
}

TEST(Utils, ToString) {
    ASSERT_EQ("1989-06-04", Utils::to_string(Date(1989, 6, 4)));
    ASSERT_EQ("1.2", Utils::to_string(1.2));
	ASSERT_EQ("0", Utils::to_string(0));
    ASSERT_EQ("-2", Utils::to_string(static_cast<signed char>(-2)));
	ASSERT_EQ("0", Utils::to_string(static_cast<signed char>(0)));
	ASSERT_EQ("0", Utils::to_string(static_cast<unsigned char>(0)));
	ASSERT_EQ("255", Utils::to_string(static_cast<unsigned char>(255)));
}


TEST(Utils, FromString) {
    ASSERT_EQ(-0.25, Utils::from_string<double>("-0.25"));
    EXPECT_EQ(Date(1989, 6, 4), Utils::from_string<Date>("1989-Jun-04"));
	EXPECT_EQ(Date(1989, 6, 4), Utils::from_string<Date>("1989-06-04"));
	EXPECT_EQ(Date(1989, 6, 4), Utils::from_string<Date>("1989/06/04"));
	EXPECT_EQ(Date(1989, 6, 4), Utils::from_string<Date>(std::string("1989-Jun-04")));
	EXPECT_EQ(Date(1989, 6, 4), Utils::from_string<Date>(std::string("1989-06-04")));
	EXPECT_EQ(Date(1989, 6, 4), Utils::from_string<Date>(std::string("1989/06/04")));
	ASSERT_EQ("AAA", Utils::from_string<std::string>("AAA"));
    ASSERT_EQ(static_cast<signed char>(-2), Utils::from_string<signed char>("-2"));
}

TEST(Utils, MakeFunction) {
	std::function<long double(long double)> myexp = Utils::make_function<long double(long double)>(std::exp);
	long double x = 0.82309483409987598324759837598L;
	averisera::testing::is_near(std::exp(x), myexp(x), 0.0L);
}


TEST(Utils, FromStringUnorderedMap) {
	std::unordered_map<std::string, std::string> map;
	map["FOO"] = "0.345";
	ASSERT_EQ(0.345, Utils::from_string_map(map, "FOO", 0.12, true));
	ASSERT_EQ(0.12, Utils::from_string_map(map, "BAR", 0.12, true));
	map["BAZZ"] = "";
	ASSERT_EQ(0.12, Utils::from_string_map(map, "BAZZ", 0.12, false));
}

