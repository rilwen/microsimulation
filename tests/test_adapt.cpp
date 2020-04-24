#include <gtest/gtest.h>
#include "core/adapt.hpp"
#include "core/normal_distribution.hpp"
#include "core/math_utils.hpp"
#include <cassert>
#include <cmath>

using namespace averisera;

#if defined (_WIN32) && !defined (_WIN64)
TEST(Adapt, Dummy) {
    std::cerr << "Adaptive integration not supported in 32-bit Windows environment" << std::endl;
}
#else

static double function_constant(const int* dim, const double* x, const int* npara, const double* params) {    
    assert(*npara == 0);
    assert(!params);
    return 1.0;
}

TEST(Adapt, Constant) {
    AdaptIntegration ai(4, 1E-12);
    double relerr;
    int ifail;
    const double result = ai.integrate(function_constant, std::vector<double>(), relerr, ifail);
    ASSERT_NEAR(1.0, result, 1E-12);
    ASSERT_EQ(0, ifail);
    ASSERT_LE(relerr, 1E-12);
    ASSERT_GT(ai.minpts(), 1u);
}

const static double biv_gauss_rho = 0.6;

static double function_gaussian(const int* dim, const double* x, const int* npara, const double* params) {
    //std::cout << npara << ", " << params << std::endl;
    assert(*dim == 2);
    assert(*npara == 1);
    assert(params[0] == biv_gauss_rho);
    static const double rho = params[0];
    static const double adj_rho = 1 - rho * rho;
    const double u = x[0];
    const double v = x[1];
    const double z1 = NormalDistribution::normsinv(u);
    const double z2 = NormalDistribution::normsinv(v);
    return exp(- (z1 * z1 + z2 * z2 - 2 * rho * z1 * z2) / 2 / adj_rho + (z1 * z1 + z2 * z2) / 2) / sqrt(adj_rho);
}

TEST(Adapt, BivariateGaussian) {
    AdaptIntegration ai(2, 1E-10, 10000);
    double relerr;
    int ifail;
    std::vector<double> params({biv_gauss_rho});
    const double result = ai.integrate(function_gaussian, params, relerr, ifail);
    ASSERT_EQ(0, ifail) << "result=" << result << ", relerr=" << relerr;
    ASSERT_NEAR(1.0, result, 1E-5) << "relerr=" << relerr;
    ASSERT_LE(relerr, 1E-10);
    ASSERT_GT(ai.minpts(), 1u);
}

static double function_custom(const int* dim, const double* x, const int* npara, const double* params) {
    assert(*dim == 3);
    assert(*npara == 2);
    return params[0] * x[0] + params[1] * x[1] * exp(x[2]);
}

TEST(Adapt, Custom) {
    AdaptIntegration ai(3, 1E-10, {0.0, 0.0, 0.0}, {0.5, 0.1, 2.0});
    double relerr;
    int ifail;
    const std::vector<double> params({0.12, 0.21});
    const double result = ai.integrate(function_custom, params, relerr, ifail);
    ASSERT_EQ(0, ifail) << result;
    const double expected = 0.12 * 0.1 * 2.0 * 0.5 * 0.5 / 2 + 0.21 * 0.5 * 0.1 * 0.1 * 0.5 * (exp(2.0) - 1.0);
    ASSERT_NEAR(expected, result, fabs(expected) * 1E-10);
}

struct data {
    double a;
    double b;
};

static double function_uses_struct(const int* dim, const double* x, const int* npara, const double* params) {
    assert(*npara == 1);
    const void * p = AdaptIntegration::get_ptr(params[0]);
    const data* d = static_cast<const data*>(p);
    return d->a + d->b;
}

TEST(Adapt, Struct) {
    AdaptIntegration ai(2, 1E-12);
    double relerr;
    int ifail;
    data d = {0.2, 0.8};
    const double result = ai.integrate(function_uses_struct, &d, relerr, ifail);
    ASSERT_NEAR(1.0, result, 1E-12);
    ASSERT_EQ(0, ifail);
    ASSERT_LE(relerr, 1E-12);
    ASSERT_GT(ai.minpts(), 1u);
}

#endif
