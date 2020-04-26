/*
(C) Averisera Ltd 2014-2020
*/
#include "cauchy_distribution.hpp"
#include "math_utils.hpp"
#include <cassert>
#include <cmath>

namespace averisera {
    double CauchyDistribution::cdf(double x) {
        return atan(x) / MathUtils::pi + 0.5;
    }
        
    double CauchyDistribution::pdf(double x) {
        return 1 / (1 + x * x) / MathUtils::pi;
    }
        
    double CauchyDistribution::icdf(double p) {
        assert(p >= 0.0);
        assert(p <= 1.0);
        return tan(0.5 * MathUtils::pi * (2 * p - 1));
    }
}
