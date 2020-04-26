// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "core/bisection.hpp"
#include <algorithm>

using namespace averisera;

static double fun(double x) {
    return x * x - 1;
}

TEST(Bisection, Test) {
    double a = 10;
    double b = 20;
    unsigned int eval;
    ASSERT_THROW(RootFinding::bisection(fun, 1E-8, 1E-8, 100, a, b, eval), std::domain_error);
    a = 0.5;
    b = 20;
    ASSERT_TRUE(RootFinding::bisection(fun, 1E-8, -1, 100, a, b, eval));
    ASSERT_LT(fabs(b - a), 1E-8) << a << ", " << b;
    ASSERT_GE(eval, 30u);
    ASSERT_LE(eval, 35u);
    a = 0.5;
    b = 20;
    ASSERT_TRUE(RootFinding::bisection(fun, -1, 1E-8, 100, a, b, eval));
    ASSERT_LT(std::min(fabs(fun(a)), fabs(fun(b))), 1E-8) << a << ", " << b;
}
