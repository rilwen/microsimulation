// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-calibrator/bivariate_gaussian_copula_calibrator.hpp"
#include "core/distribution_linear_interpolated.hpp"
#include "core/distribution_shifted_lognormal.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(BivariateGaussianCopulaCalibrator, CalculateRho) {
    const auto x = std::make_shared<DistributionShiftedLognormal>(1.0, 0.5, -0.2);
    const auto y = std::make_shared<DistributionLinearInterpolated>(std::vector<double>({0.0, 1.0}), std::vector<double>({1.0}));
    BivariateGaussianCopulaCalibrator calibr(1E-10, 1000);
    ASSERT_NEAR(0.0, calibr.calculate_rho(0.0, x, y), 3E-10);
    ASSERT_GT(calibr.calculate_rho(0.01, x, y), 0.0);
    ASSERT_LT(calibr.calculate_rho(-0.01, x, y), 0.0);
    ASSERT_NEAR(calibr.calculate_rho(1, x, y), -calibr.calculate_rho(-1, x, y), 1E-12);
    std::cout << "Maximum achievable correlation: " << calibr.calculate_rho(1, x, y) << std::endl;
    std::cout << "Minimum achievable correlation: "<< calibr.calculate_rho(-1, x, y) << std::endl;
}

TEST(BivariateGaussianCopulaCalibrator, Calibrate) {
    const auto x = std::make_shared<DistributionShiftedLognormal>(1.0, 0.5, -0.2);
    const auto y = std::make_shared<DistributionLinearInterpolated>(std::vector<double>({0.0, 1.0, 2.0}), std::vector<double>({.4, .6}));
    BivariateGaussianCopulaCalibrator calibr(1E-10, 1000);
    ASSERT_EQ(0.0, calibr.calibrate(0, x, y));
    ASSERT_GT(calibr.calibrate(0.6, x, y), 0.0);
    ASSERT_LT(calibr.calibrate(-0.61, x, y), 0.0);
}
