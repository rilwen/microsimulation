#pragma once
#include "distribution.hpp"
#include "preconditions.hpp"
#include "running_statistics.hpp"
#include <boost/math/distributions/beta.hpp>
#include <boost/format.hpp>

namespace averisera {
    /** Beta distribution */
    class BetaDistribution: public Distribution {
    public:
        /** @param alpha Alpha parameter of the distribution, > 0
		@param beta Beta parameter of the distribution, > 0
		@param x0 Left end of the range
        @param x1 Right end of the range, x1 > x0 so that (x1 - x0) > 0.0 numerically. (x0 - x1) must be finite.
		@throw std::domain_error If parameters do not fulfill the above bounds.
        */
        BetaDistribution(double alpha, double beta, double x0 = 0.0, double x1 = 1.0);
		BetaDistribution(const BetaDistribution& other) = default;

        double pdf(double x) const override;

        double cdf(double x) const override;

        double icdf(double p) const override;

		double mean() const override {
			return _mean;
		}

		double variance(double /*mean*/) const override {
			return _variance;
		}

		double alpha() const {
			return _distr01.alpha();
		}

		double beta() const {
			return _distr01.beta();
		}

		double infimum() const override {
			return _x0;
		}

		double supremum() const override {
			return _x1;
		}

		/** Estimate given mean and variance on range [x0, x1] (method of moments).
		@param mean Required mean. x0 < mean < x1
		@param variance Required variance. variance < (mean - x0) * (x1 - mean)
		@param x0 Left end of the range
		@param x1 Right end of the range, x1 > x0 so that (x1 - x0) > 0.0 numerically. (x0 - x1) must be finite.
		@throw std::domain_error If parameters do not fulfill the above bounds.
		*/
		static BetaDistribution estimate_given_range(double mean, double variance, double x0, double x1);

		/** Estimate from sample given a range [x0, x1] (method of moments).
		@param x0 Left end of the range
		@param x1 Right end of the range, x1 > x0 so that (x1 - x0) > 0.0 numerically. (x0 - x1) must be finite.
		@throw std::domain_error If x0, x1 parameters do not fulfill the above bounds.
		@throw std::runtime_error If sample cannot be described with a Beta distribution supported on [x0, x1] range.
		*/
		template <class I> static BetaDistribution estimate_given_range(I begin, I end, double x0, double x1);

		/** Estimate from sample using the (method of moments). Set the range to [min(sample), max(sample)].
		@throw std::runtime_error If sample cannot be described with a Beta distribution supported on [x0, x1] range.
		*/
		template <class I> static BetaDistribution estimate(I begin, I end);
    private:
		boost::math::beta_distribution<double> _distr01;
        double _x0;
        double _x1;
		double _width; /**< Width of the support */
		double _mean;
		double _variance;

		double to01(double x) const;

		// return width
		static double validate_range(double x0, double x1);
    };

	template <class I> BetaDistribution BetaDistribution::estimate_given_range(I begin, const I end, double x0, double x1) {
		//const double width = validate_range(x0, x1);
		RunningStatistics<double> rs;
		while (begin != end) {
			rs.add_if_not_nan(*begin);
			++begin;
		}
		if (rs.min() < x0 || rs.max() > x1) {
			throw std::runtime_error("BetaDistribution: sample values outside range");
		}
		try {
			return estimate_given_range(rs.mean(), rs.variance(), x0, x1);
		} catch (std::domain_error& e) {
			throw std::runtime_error(boost::str(boost::format("BetaDistribution: cannot estimate from sample: %s") % e.what()));
		}
	}

	template <class I> BetaDistribution BetaDistribution::estimate(I begin, const I end) {
		RunningStatistics<double> rs;
		while (begin != end) {
			rs.add_if_not_nan(*begin);
			++begin;
		}
		try {
			return estimate_given_range(rs.mean(), rs.variance(), rs.min(), rs.max());
		} catch (std::domain_error& e) {
			throw std::runtime_error(boost::str(boost::format("BetaDistribution: cannot estimate from sample: %s") % e.what()));
		}
	}
}
