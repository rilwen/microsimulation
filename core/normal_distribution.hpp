// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_NORMAL_DISTRIBUTION_H
#define __AVERISERA_NORMAL_DISTRIBUTION_H

#include "distribution.hpp"
#include "preconditions.hpp"
#include "running_statistics.hpp"
#include <cmath>
#include <utility>
#include <type_traits>

namespace averisera {
	//
	// High-precision normal distribution functions
	//
	class NormalDistribution: public Distribution {
	public:
		/**
		@param mean mean of the distribution
		@param sigma standard deviation
		@throw std::domain_error If mu or sigma is not finite. If sigma is negative.
		*/
		NormalDistribution(double mean = 0, double sigma = 1)
			: NormalDistribution(mean, sigma, true) {}

		/** Implementation of constructor
		@param validate Validate inputs?
		@throw std::domain_error (Only if validate == true) If mu or sigma is not finite. If sigma is negative.
		*/
		NormalDistribution(double mean, double sigma, bool validate);

		NormalDistribution(const NormalDistribution& other);
		double pdf(double x) const override { return normpdf(x, _mean, _sigma); }
		double cdf(double x) const override { return normcdf( (x - _mean) / _sigma); }
		double icdf(double p) const override;

		// ERF function; most precise around 0
		inline static double erf(double x)
		{
			return calerf(x, false);
		}

		// ERFC function; most precise in the tails.
		inline static double erfc(double x)
		{
			return calerf(x, true);
		}

		// Normal density function.
		inline static double normpdf(double x)
		{
			return 0.398942280401433 * std::exp(-0.5*x*x);
		}

		inline static double normpdf(double x, double mean, double sigma)
		{
			return normpdf((x - mean)/sigma)/sigma;
		}

		// Normal Gaussian CDF
		inline static double normcdf(const double x)
		{
			// This ensures max precision.
			return 0.5*erfc(-0.7071067811865475244008443621 * x);
		}

		// Inverse normal Gaussian CDF
		double static normsinv(double p);

        double mean() const override {
            return _mean;
        }

        double variance(double /*mean*/) const override {
            return _sigma * _sigma;
        }

		double sigma() const {
			return _sigma;
		}

		double infimum() const override {
			return _sigma > 0 ? (-std::numeric_limits<double>::infinity()) : _mean;
		}

		double supremum() const override {
			return _sigma > 0 ? (std::numeric_limits<double>::infinity()) : _mean;
		}

        std::unique_ptr<Distribution> clone() const;

        double conditional_mean(double a, double b) const override;

		/** Estimate (mu, variance) from a data sample in the [begin, end) iterator range.
		@param unbiased If true, use the unbiased estimator of variance. 
        @tparam V Value type
        */
        template <class V, class I> static std::pair<V, V> estimate_params(I begin, I end, bool unbiased);
        
		/** Estimate NormalDistribution from a data sample in the [begin, end) iterator range.
		@param unbiased If true, use the unbiased estimator of variance. */
		template <class I> static NormalDistribution estimate(I begin, I end, bool unbiased);
	private:
		double _mean;
		double _sigma;

		// Don't call this directly unless you know what you're doing.
		static double calerf(const double x, const bool calculate_erfc);
	};

    template <class V, class I> std::pair<V, V> NormalDistribution::estimate_params(I begin, I end, bool unbiased) {
        RunningStatistics<V> rs;
		while (begin != end) {
			rs.add_if_finite(*begin);
			++begin;
		}
        V var = rs.variance();
        if (!unbiased) {
            const V n = static_cast<V>(static_cast<double>(rs.nbr_samples()));
            var *= (n - 1.0) / n;
        }
        return std::make_pair(rs.mean(), var);
    }

	template <class I> NormalDistribution NormalDistribution::estimate(I begin, const I end, bool unbiased) {
		const auto mu_var = estimate_params<typename std::remove_const<typename std::remove_reference<decltype(*begin)>::type>::type, I>(begin, end, unbiased);
		return NormalDistribution(mu_var.first, sqrt(mu_var.second));
	}
}

#endif 
