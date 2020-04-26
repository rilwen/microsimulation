/*
(C) Averisera Ltd 2014-2020
*/
#include "beta_distribution.hpp"
#include <stdexcept>

namespace averisera {
	BetaDistribution::BetaDistribution(double alpha, double beta, double x0, double x1)
		: _distr01(alpha, beta), _x0(x0), _x1(x1), _width(validate_range(x0, x1)) {
		check_that(alpha > 0.0, "Alpha must be positive");
		check_that(beta > 0.0, "Beta must be positive");
		_mean = _x0 + _width * alpha / (alpha + beta);
		_variance = pow(_width, 2) * alpha * beta / pow(alpha + beta, 2) / (alpha + beta + 1);
	}

	double BetaDistribution::pdf(double x) const {
		return boost::math::pdf(_distr01, to01(x)) / _width;
	}

	double BetaDistribution::cdf(double x) const {
		return boost::math::cdf(_distr01, to01(x));
	}

	double BetaDistribution::icdf(double p) const {
		// boost does checks for p
		return boost::math::quantile(_distr01, p) * _width + _x0;
	}

	double BetaDistribution::to01(double x) const {
		return (x - _x0) / _width;
	}

	double BetaDistribution::validate_range(double x0, double x1) {
		check_that(x1 > x0, "X1 must be larger than X0");
		const double width = x1 - x0;
		check_that(width > 0, "X1 too close to X0");
		check_that(std::isfinite(width), "[X0, X1] range must be finite");
		return width;
	}

	BetaDistribution BetaDistribution::estimate_given_range(const double mean, const double variance, const double x0, const double x1) {
		const double width = validate_range(x0, x1);
		check_that(mean > x0, "Mean must be larger than X0");
		check_that(mean < x1, "Mean must be smaller than X0");
		check_that(variance < (mean - x0) * (x1 - mean), "Variance must be smaller than (Mean - X0) * (X1 - Mean)");		
		const double mean01 = (mean - x0) / width;
		const double variance01 = variance / pow(width, 2);
		const double alpha = boost::math::beta_distribution<double>::find_alpha(mean01, variance01);
		const double beta = boost::math::beta_distribution<double>::find_beta(mean01, variance01);
		return BetaDistribution(alpha, beta, x0, x1);
	}
}
