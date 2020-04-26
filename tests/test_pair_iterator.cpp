// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/pair_iterator.hpp"
#include <algorithm>
#include <vector>

using namespace averisera;

TEST(PairIterator, Const) {
    const std::vector<double> d({0.2, 0.3});
    const std::vector<char> c({'a', 'z'});
    auto i1 = make_pair_iterator(d.begin(), c.begin());
    ASSERT_EQ(std::make_pair(0.2, 'a'), *i1);
    ASSERT_EQ(0.2, *i1.first());
    ASSERT_EQ('a', *i1.second());
    PairConstIterator<std::vector<double>::const_iterator, std::vector<char>::const_iterator> i2(d.end(), c.end());
    ASSERT_TRUE(i1 != i2);
    ASSERT_TRUE(i1 <= i2);
    ASSERT_TRUE(i1 < i2);
    ++i1;
    ASSERT_EQ(std::make_pair(0.3, 'z'), *i1);    
    --i1;
    ASSERT_EQ(std::make_pair(0.2, 'a'), *i1);
    ++i1; ++i1;
    ASSERT_TRUE(i1 == i2);
    ASSERT_TRUE(i1 <= i2);
}

TEST(PairIterator, UpperBound) {
    const std::vector<double> d({0.2, 0.3, 0.4});
    const std::vector<char> c({'a', 'b', 'c'});
    auto comparison = [](double x, std::pair<double, char> p) { return x < p.first; };
    auto i1 = make_pair_iterator(d.begin(), c.begin());
    auto i2 = make_pair_iterator(d.end(), c.end());
    const auto r1 = std::upper_bound(i1, i2, 0.3, comparison);
    ASSERT_EQ(0.4, (*r1).first);
    ASSERT_EQ('c', (*r1).second);
    const auto r2 = std::upper_bound(i1, i2, 0.4, comparison);
    ASSERT_LT(r1, r2);
    ASSERT_EQ(r2, i2);
}
