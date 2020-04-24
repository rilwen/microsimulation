/*
  (C) Averisera Ltd 2014
  Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_DISCRETE_DISTRIBUTION_H
#define __AVERISERA_DISCRETE_DISTRIBUTION_H

#include "distribution.hpp"
#include "generic_distribution.hpp"
#include "math_utils.hpp"
#include "preconditions.hpp"
#include <vector>
#include <cassert>
#include <cmath>

namespace averisera {
    // Represents a discrete distribution on a range [a, a + 1, ..., b - 1, b]
    class DiscreteDistribution: public Distribution, public GenericDistribution<int> {
    public:
        // Construct a distribution with values a, a+1, a+2, ..., a+N-1
        // a: 1st value
        // p: Probability vector (anything which has .size() method and [] operator)
        template <class V> DiscreteDistribution(int a, const V& p);

        /** Constructor which moves probabilities */
        DiscreteDistribution(int a, std::vector<double>&& p);

        // Construct a distribution choosing q from [a, b] with certainty
        // a: 1st value
        // q: 100% probable value, a <= q <= b
        // b: Last value
        DiscreteDistribution(int a, int q, int b);

        // Default: value 0 with probability 1
        DiscreteDistribution();

        DiscreteDistribution(const DiscreteDistribution&) = default;
        DiscreteDistribution& operator=(const DiscreteDistribution&) = default;

        /** Move constructor */
        DiscreteDistribution(DiscreteDistribution&& other);

		/** Assign new probabilities
		@throw std::domain_error If p.size() different than size() */
		void assign_proba(const std::vector<double>& p);

        double pdf(double x) const override;
        double cdf(double x) const override;
        double cdf2(double x) const override;
        double icdf(double p) const override;
	    

        // Inverse CDF with integer value returned.
        int icdf_generic(double p) const override { 
            return static_cast<int>(icdf(p)); 
        }

		using Distribution::draw;

        int random(RNG& rng) const override {
            return static_cast<int>(draw(rng));
        }

        using Distribution::range_prob2;

        double range_prob2(int x1, int x2) const override {            
            return range_prob2(static_cast<double>(x1), static_cast<double>(x2));
        }

        // Return size := b - a + 1
        unsigned int size() const { return static_cast<unsigned int>(_p.size()); }

        // Return probability X == k
        // k: value from [a, b]
        double prob(int k) const { return _p[static_cast<size_t>(k - _a)]; }

        double mean() const override;

        double variance(double mean) const override;

        double conditional_mean(double a, double b) const override;

        double conditional_variance(double conditional_mean, double a, double b) const override;

        /*std::unique_ptr<Distribution> clone() const;*/

        DiscreteDistribution* conditional(int left, int right) const override;

        int lower_bound() const override {
            return _a;
        }

        int upper_bound() const override {
            return _b;
        }

		double infimum() const override {
			return _a;
		}

		double supremum() const override {
			return _b;
		}

		/** Draw from a discrete distribution given a U(0, 1) random variable value u and 
		[begin, end) range of iterators containing CDF values. The last CDF value is assumed to be 1.0*/
		template <class I> static size_t draw_from_cdf(const I begin, const I end, double u) {
			const auto it = std::upper_bound(begin, end, u);
			const auto dim = std::distance(begin, end);
			assert(dim >= 0);
			const auto next_state = (it != end) ?
				std::distance(begin, it)
				:
				(dim - 1);
			return static_cast<size_t>(next_state);
		}
    private:
        int _a;
        int _b;
        std::vector<double> _p;
        std::vector<double> _cum_p;

		void calc_cum_p() {
			calculate_cumulative_proba(_p, _cum_p, 1e-8);
		}
    };

    template <class V> DiscreteDistribution::DiscreteDistribution(const int a, const V& p)
        : _a(a), _b(MathUtils::safe_cast<int>((a + p.size()) - 1)), _p(p.size()), _cum_p(p.size())
    {
        const size_t dim = p.size();
		for (size_t i = 0; i < dim; ++i) {
			_p[i] = p[i];
		}
		calc_cum_p();
    }
}

#endif
