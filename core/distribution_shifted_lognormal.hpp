#ifndef __AVERISERA_DISTR_SHFT_LOGN_H
#define __AVERISERA_DISTR_SHFT_LOGN_H

#include "distribution.hpp"
#include "normal_distribution.hpp"
#include <array>
#include <cassert>
#include <cmath>
#define BOOST_RESULT_OF_USE_DECLTYPE
#include <boost/iterator/transform_iterator.hpp>

namespace averisera {
    /** @brief Shifted log-normal distribution.

      Distribution of Y = A + exp(X), where X ~ N(mu, sigma) and A is the shift.

    */
    class DistributionShiftedLognormal: public Distribution {
    public:
		/**
		@throw std::domain_error If mu, sigma or shift is not finite. If sigma is negative. 
		*/
        DistributionShiftedLognormal(double mu, double sigma, double shift);

		DistributionShiftedLognormal(const DistributionShiftedLognormal& other);
	
        double pdf(double y) const;
	
        double cdf(double y) const {
            assert(y >= _shift);
            return _normal.cdf(log(y - _shift));
        }
	
        double icdf(double p) const {
			const double x = _normal.icdf(p);
            return _shift + exp(x);
        }

        double mean() const override {
            return _mean;
        }

        double variance(double mean) const override {
            return _variance;
        }

		double log_mean() const {
			return _normal.mean();
		}

		double log_sigma() const {
			return _normal.sigma();
		}

		double shift() const {
			return _shift;
		}

		/** Median of the distribution */
		double median() const {
			return median(_normal.mean(), _shift);
		}

		/** Median of shifted log-normal distribution with given shift and Gaussian parameter mu. */
		template <class T> static T median(T mu, T shift) {
			return exp(mu) + shift;
		}

		double infimum() const override {
			return _shift;
		}

		double supremum() const override {
			return std::numeric_limits<double>::infinity();
		}

		/** Estimate log-normal distribution with given shift using ML */
        template <class V, class I> static std::pair<V, V> estimate_params_given_shift(I begin, I end, V shift);

		/** Estimate log-normal distribution with given shift using ML */
		template <class I> static DistributionShiftedLognormal estimate_given_shift(I begin, I end, double shift = 0.0);

        /** Estimate log-normal distribution with unknown shift using the method from R. J. Aristizabal, "Estimating the Parameters of the Three-Parameter
          Lognormal Distribution". Reorders elements of sample.
          @throw std::domain_error If sample.size() < 10.
         */
        static DistributionShiftedLognormal estimate(std::vector<double>& sample);

		/**
		Estimate given a shift and p0 = P(X < x0) and p1 = P(x0 <= x < x1). p0 + p1 < 1
		@param p0 > 0
		@param p1 > 0
		@throw std::domain_error If parameters do not allow an exact fit
		*/
		static DistributionShiftedLognormal fit_exactly_given_shift(double p0, double p1, double shift, double x0, double x1);

		/**
		Find a shifted lognormal distribution which approximates the discrete distribution p[k] = P(x[k] < X < x[k+1]) the closest
		(as measured by K-L divergence), with the shift given by x[0] and x[x.size()] == infinity.
		*/
		static DistributionShiftedLognormal estimate_given_shift(const std::vector<double>& x, const std::vector<double>& p);
    private:
        NormalDistribution _normal;
        double _shift;
        double _mean;
        double _variance;
    };

    template <class V, class I> std::pair<V, V> DistributionShiftedLognormal::estimate_params_given_shift(I begin, I end, V shift) {
		auto transform = [shift](double x) -> V { return log(x - shift); };
        const auto log_begin = boost::make_transform_iterator(begin, transform);
        const auto log_end = boost::make_transform_iterator(end, transform);
		return NormalDistribution::estimate_params<V>(log_begin, log_end, false);
	}

	template <class I> DistributionShiftedLognormal DistributionShiftedLognormal::estimate_given_shift(I begin, I end, const double shift) {
		const auto mu_var = estimate_params_given_shift<double>(begin, end, shift);
		return DistributionShiftedLognormal(mu_var.first, sqrt(mu_var.second), shift);
	}
}

#endif // __AVERISERA_DISTR_SHFT_LOGN_H
