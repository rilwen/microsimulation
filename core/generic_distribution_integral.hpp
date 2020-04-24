/*
  (C) Averisera Ltd 2015
  Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_GENERIC_DISTRIBUTION_INTEGRAL_H
#define __AVERISERA_GENERIC_DISTRIBUTION_INTEGRAL_H

#include "distribution.hpp"
#include "generic_distribution.hpp"
#include "kahan_summation.hpp"
#include "math_utils.hpp"
#include "preconditions.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <type_traits>
#include <vector>

namespace averisera {
    /** Represents a discrete distribution on a range [a, a + 1, ..., b - 1, b]
	@tparam T integral type */
    template <class T> class GenericDistributionIntegral: public GenericDistribution<typename std::enable_if<std::is_integral<T>::value, T>::type> {
    public:
        // Construct a distribution with values a, a+1, a+2, ..., a+N-1
        // a: 1st value
        // p: Probability vector (anything which has .size() method and [] operator)
        template <class V> GenericDistributionIntegral(T a, const V& p);

        /** Constructor which moves probabilities */
        GenericDistributionIntegral(T a, std::vector<double>&& p);

        // Construct a distribution choosing q from [a, b] with certainty
        // a: 1st value
        // q: 100% probable value, a <= q <= b
        // b: Last value
        GenericDistributionIntegral(T a, T q, T b);

        // Default: value 0 with probability 1
        GenericDistributionIntegral();

        GenericDistributionIntegral(const GenericDistributionIntegral<T>&) = default;
        GenericDistributionIntegral& operator=(const GenericDistributionIntegral<T>&) = default;

        /** Move constructor */
        GenericDistributionIntegral(GenericDistributionIntegral<T>&& other);

        // Inverse CDF with integer value returned.
        T icdf_generic(double p) const override;

        T random(RNG& rng) const override {
            return icdf_generic(rng.next_uniform());
        }

        double range_prob2(T x1, T x2) const override {            
            if (x2 <= x1) {
                return 0.;
            } else {
                return cdf2(x2) - cdf2(x1);
            }
        }

        GenericDistributionIntegral<T>* conditional(T left, T right) const override;

        T lower_bound() const override {
            return _a;
        }

        T upper_bound() const override {
            return _b;
        }

        /** Calculate P(X < x)
          (Note the sharp inequality)
        */
        double cdf2(T x) const;

        // Return probability X == k
        // k: value from [a, b]
        double prob(T k) const { return _p[static_cast<size_t>(k - _a)]; }

        const std::vector<double>& probs() const {
            return _p;
        }

        /** Length of probability vector */
        unsigned int size() const {
            return static_cast<unsigned int>(_p.size());
        }

		/** Calculate the mean. Constraints on T ensure that integral_distr has discrete values */
		static double mean(const GenericDistribution<T>& integral_distr);
    private:
        T _a;
        T _b;
        std::vector<double> _p;
        std::vector<double> _cum_p;
    };

    template <class T> template <class V> GenericDistributionIntegral<T>::GenericDistributionIntegral(const T a, const V& p)
        : _a(a), _b(static_cast<T>((a + p.size()) - 1)), _p(p.size()), _cum_p(p.size())
    {
        const size_t dim = p.size();
        assert(dim > 0);
        _p[0] = p[0];
        _cum_p[0] = _p[0];
        assert(_p[0] >= 0 && _p[0] <= 1);
        for (size_t i = 1; i < dim; ++i) {
            _p[i] = p[i];
            assert(_p[i] >= 0 && _p[i] <= 1);
            _cum_p[i] = _cum_p[i - 1] + _p[i];
        }
        assert(std::fabs(_cum_p.back() - 1.0) < 1E-8);
        _cum_p[dim - 1] = 1.0;
    }

    template <class T> GenericDistributionIntegral<T>::GenericDistributionIntegral(const T a, const T q, const T b)
		: _a(a), _b(b), _p(b - a + 1, 0.0), _cum_p(b - a + 1)
	{
		check_that(a <= q);
		check_that(q <= b);
		_p[q - a] = 1.0;
		std::fill(_cum_p.begin() + (q - a), _cum_p.end(), 1.0);
	}

	template <class T> GenericDistributionIntegral<T>::GenericDistributionIntegral()
		: _a(0), _b(0), _p(1, 1.0), _cum_p(1, 1.0)
	{}

    template <class T> GenericDistributionIntegral<T>::GenericDistributionIntegral(T a, std::vector<double>&& p)
        : _a(a),
          _b(static_cast<T>((a + p.size()) - 1)),
          _p(std::move(p)), // now p is undefined
          _cum_p(_p.size()) {
        p.resize(0);
        const size_t dim = _p.size();
        assert(dim > 0);
        _cum_p[0] = _p[0];
        assert(_p[0] >= 0 && _p[0] <= 1);
        for (size_t i = 1; i < dim; ++i) {
            assert(_p[i] >= 0 && _p[i] <= 1);
            _cum_p[i] = _cum_p[i - 1] + _p[i];
        }
        assert(std::fabs(_cum_p.back() - 1.0) < 1E-8);
        _cum_p[dim - 1] = 1.0;
    }

    template <class T> GenericDistributionIntegral<T>::GenericDistributionIntegral(GenericDistributionIntegral<T>&& other)
        : _a(other._a), _b(other._b),
          _p(std::move(other._p)),
          _cum_p(std::move(other._cum_p)) {
        other._p.resize(0);
        other._cum_p.resize(0);
    }

    template <class T> T GenericDistributionIntegral<T>::icdf_generic(const double p) const {
		if (p < 0 || p > 1) {
			throw std::domain_error("Probability outside [0, 1] range");
		}
		if (p == 0) {
			return _a;
		} else if (p == 1) {
			return _b;
		} else {
			T i = static_cast<T>(Distribution::idx(_cum_p, p));
			if (p == _cum_p[i]) {
				++i; // account for the case of X == x in CDF(x) := P(X <= x)
			}
			return MathUtils::safe_cast<T>(_a + i); // enforce addition with sign
		}
	}

    template <class T> GenericDistributionIntegral<T>* GenericDistributionIntegral<T>::conditional(T left, T right) const {
        const double p_ab = range_prob2(left, right);
        if (p_ab == 0) {
            throw std::runtime_error("GenericDistributionIntegral: zero probability for conditional range");
        }
        assert(right >= _a);
        assert(left <= _b + 1);
        left = std::max(left, _a);
        // right = std::min<T>(right, static_cast<T>(_b + 1));
        if (right > _b) {
            // if right > _b, then right >= _b + 1 and min(right, _b + 1) == _b + 1
            right = static_cast<T>(_b + 1); // cast is safe because right is a value in T
        } // else right is certainly less than _b + 1
        assert(right > left);
        std::vector<double> new_probs(_p.begin() + (left - _a), _p.begin() + (right - _a));
        std::for_each(new_probs.begin(), new_probs.end(), [p_ab](double& np){ np = std::min(1.0, std::max(0.0, np / p_ab)); });
        return new GenericDistributionIntegral<T>(left, std::move(new_probs));
    }

    template <class T> double GenericDistributionIntegral<T>::cdf2(T x) const {        
		if (x <= _a) {
			return 0;
		} else {
			return _cum_p[static_cast<size_t>(std::min(static_cast<T>(x - 1), _b) - _a)];
		}
	}

	template <class T> double GenericDistributionIntegral<T>::mean(const GenericDistribution<T>& integral_distr) {
		const T a = integral_distr.lower_bound();
		const T b = integral_distr.upper_bound();		
		KahanSummation<double> sumpx;
		if (b < std::numeric_limits<T>::max()) {			
			for (T x = a; x <= b; ++x) {
				const double p = integral_distr.range_prob2(x, static_cast<T>(x + 1));
				sumpx += static_cast<double>(x) * p;
			}
		} else {
			KahanSummation<double> sump;
			for (T x = a; x < b; ++x) {
				const double p = integral_distr.range_prob2(x, static_cast<T>(x + 1));
				sump += p;
				sumpx += static_cast<double>(x) * p;
			}
			sumpx += static_cast<double>(b) * std::max(1.0 - sump, 0.0);
		}			
		return sumpx;
	}
}

#endif
