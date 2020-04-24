/*
(C) Averisera Ltd 2014-17
*/
#ifndef __AVERISERA_DISTR_LIN_INTERP_H
#define __AVERISERA_DISTR_LIN_INTERP_H

#include "distribution.hpp"
#include "segment_search.hpp"
#include <numeric>
#include <vector>
#include <boost/format.hpp>

namespace averisera {
    /** @brief Distribution with linearly interpolated CDF and piecewise-constant PDF
     */
    class DistributionLinearInterpolated: public Distribution {
    public:
		DistributionLinearInterpolated(const std::vector<double>& x, const std::vector<double>& p)
			: DistributionLinearInterpolated(std::vector<double>(x), std::vector<double>(p)) {}
	
        /** Constructor which moves the values and probabilities.
          @param[in] x Vector of range boundaries
          @param[in] p Vector of probabilities for each range
          @throw std::domain_error If x.size() != p.size() + 1, p.size() < 1, any p[i] < 0 or sum_i p[i] != 1.0 within numerical precision.
        */
        DistributionLinearInterpolated(std::vector<double>&& x, std::vector<double>&& p);

        /** Move constructor */
        DistributionLinearInterpolated(DistributionLinearInterpolated&& other);

        /** Copy constructor */
        DistributionLinearInterpolated(const DistributionLinearInterpolated& other);

        DistributionLinearInterpolated& operator=(const DistributionLinearInterpolated& other) = default;

        double pdf(double x) const override;

        double cdf(double x) const override;

        double icdf(double x) const override;

        double mean() const override {
            return _mean;
        }

        double variance(double) const override {
            return _variance;
        }

        std::unique_ptr<Distribution> clone() const;

		/** Estimate the distribution by counting the numbers of [begin, end) sample points in each range
		@tparam I Iterator type
		@param begin Begin iterator 
		@param end End iterator
		@param boundaries Range boundaries in strictly ascending order.
		@param check_range If false, silently discard values outside range of boundaries.
		@param check_nan If false, silently discard NaN value.
		@return Estimated distribution
		@throw std::domain_error If boundaries.size() < 2.
		@throw std::runtime_error If check_range == true and some value in data is outside boundaries. If check_nan == true any value in data is NaN.
		*/
		template <class I> static DistributionLinearInterpolated estimate(I begin, I end, std::vector<double>&& boundaries, bool check_range, bool check_nan);

		size_t nbr_ranges() const {
			return _p.size();
		}

		bool operator==(const DistributionLinearInterpolated& other) const;

		double infimum() const override {
			return _x.front();
		}

		double supremum() const override {
			return _x.back();
		}
    private:
        std::vector<double> _x; /**< Range boundaries */
        std::vector<double> _p; /**< Probabilities for each range */
        std::vector<double> _cp; /**< Cumulative probabilities */
        double _mean;
        double _variance;
    };

	template <class I> DistributionLinearInterpolated DistributionLinearInterpolated::estimate(I begin, const I end, std::vector<double>&& boundaries, const bool check_range, const bool check_nan) {
		const size_t N = boundaries.size();
		if (N < 2) {
			throw std::domain_error("DistributionLinearInterpolated: boundaries vector too small");
		}
		std::vector<size_t> counts(N - 1, 0);
		for (; begin != end; ++begin) {
			const double x = *begin;
			if (std::isnan(x)) {
				if (check_nan) {
					throw std::runtime_error("DistributionLinearInterpolated: NaN value detected");
				} else {
					continue;
				}
			}
			const size_t idx = SegmentSearch::binary_search_left_inclusive(boundaries, x);
			if (idx >= counts.size()) {
				if (check_range) {
					throw std::runtime_error(boost::str(boost::format("DistributionLinearInterpolated: value out of range: %g") % x)); ;
				} else {
					continue;
				}
			}
			++counts[idx];
		}
		const double total = static_cast<double>(std::accumulate(counts.begin(), counts.end(), static_cast<size_t>(0)));
		std::vector<double> probs(N - 1);
		std::transform(counts.begin(), counts.end(), probs.begin(), [total](size_t cnt) {return static_cast<double>(cnt) / total; });
		return DistributionLinearInterpolated(std::move(boundaries), std::move(probs));
	}
}

#endif // __AVERISERA_DISTR_LIN_INTERP_H
