/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "discrete_distribution.hpp"
#include "log.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

namespace averisera {
	

	DiscreteDistribution::DiscreteDistribution(const int a, const int q, const int b)
		: _a(a), _b(b), _p(b - a + 1, 0.0), _cum_p(b - a + 1, 0.0)
	{
		check_that(a <= q);
		check_that(q <= b);
		_p[q - a] = 1.0;
		std::fill(_cum_p.begin() + (q - a), _cum_p.end(), 1.0);
	}

	DiscreteDistribution::DiscreteDistribution()
		: _a(0), _b(0), _p(1, 1.0), _cum_p(1, 1.0)
	{}

    DiscreteDistribution::DiscreteDistribution(int a, std::vector<double>&& p)
        : _a(a),
          _b(MathUtils::safe_cast<int>((a + p.size()) - 1)),
          _p(std::move(p)), // now p is undefined
          _cum_p(_p.size()) {
        p.resize(0);
		calc_cum_p();
    }

    DiscreteDistribution::DiscreteDistribution(DiscreteDistribution&& other)
        : _a(other._a), _b(other._b),
          _p(std::move(other._p)),
          _cum_p(std::move(other._cum_p)) {
        other._p.resize(0);
        other._cum_p.resize(0);
    }

	double DiscreteDistribution::pdf(double x) const {
		// poor man's Dirac delta
		LOG_DEBUG() << "DiscreteDistribution: calculating continuous PDF for discrete distribution";
		if (x == round(x)) {
			const int i = static_cast<int>(x);			
			if (i >= _a && i <= _b) {
				return std::numeric_limits<double>::infinity();
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	}

	double DiscreteDistribution::cdf(double x) const {
		const int i = static_cast<int>(std::floor(x));
		if (i < _a) {
			return 0;
		} else {
			return _cum_p[std::min(i, _b) - _a];
		}
	}

    double DiscreteDistribution::cdf2(double x) const {        
		if (x <= _a) {
			return 0;
		} else {
            const double fla = std::floor(x);
            int i = static_cast<int>(fla);
            if (x == fla) {
                --i;
            }
			return _cum_p[std::min(i, _b) - _a];
		}
	}

	double DiscreteDistribution::icdf(const double p) const {
		if (p < 0 || p > 1) {
			throw std::out_of_range("DiscreteDistribution: Probability outside [0, 1] range");
		}
		if (p == 0) {
			return _a;
		} else if (p == 1) {
			return _b;
		} else {
			int i = static_cast<int>(idx(_cum_p, p));
			if (p == _cum_p[i]) {
				++i; // account for the case of X == x in CDF(x) := P(X <= x)
			}
			return _a + i; // enforce addition with sign
		}
	}

    double DiscreteDistribution::mean() const {
        int k = _a;
        double sum = 0;
        for (auto it = _p.begin(); it != _p.end(); ++it, ++k) {
            sum += k * (*it);       
        }
        assert(k == _b + 1);
        return sum;
    }

    double DiscreteDistribution::variance(const double mean) const {
        int k = _a;
        double sum = 0;
        for (auto it = _p.begin(); it != _p.end(); ++it, ++k) {
            const double x = k - mean;
            sum += x * x  * (*it);       
        }
        assert(k == _b + 1);
        return sum;
    }

    double DiscreteDistribution::conditional_mean(const double a, const double b) const {
        if (b <= a) {
            throw std::domain_error("DiscreteDistribution: b <= a");
        }
        const int i2 = std::min(_b + 1, static_cast<int>(std::ceil(b))) - _a;
        const int i1 = std::max(_a, static_cast<int>(std::ceil(a))) - _a;
        double sum_p = 0;
        double sum_xp = 0;
        for (int i = i1; i < i2; ++i) {
            sum_p += _p[i];
            sum_xp += _p[i] * (i + _a);
        }
        return sum_xp / sum_p;
    }

    double DiscreteDistribution::conditional_variance(const double conditional_mean, const double a, const double b) const {
        if (b <= a) {
            throw std::domain_error("DiscreteDistribution: b <= a");
        }
        if (conditional_mean < a || conditional_mean >= b) {
            throw std::domain_error("DiscreteDistribution: conditional_mean outside [a, b)");
        }
        const int i2 = std::min(_b + 1, static_cast<int>(std::ceil(b))) - _a;
        const int i1 = std::max(_a, static_cast<int>(std::ceil(a))) - _a;
        double sum_p = 0;
        double sum_xp = 0;
        for (int i = i1; i < i2; ++i) {
            sum_p += _p[i];
            sum_xp += _p[i] * pow(i + _a - conditional_mean, 2);
        }
        return sum_xp / sum_p;        
    }

    /*std::unique_ptr<Distribution> DiscreteDistribution::clone() const {
        return std::unique_ptr<Distribution>(new DiscreteDistribution(*this));
    }*/

    DiscreteDistribution* DiscreteDistribution::conditional(int left, int right) const {
        const double p_ab = range_prob2(left, right);
        if (p_ab == 0) {
            throw std::runtime_error("DiscreteDistribution: zero probability for conditional range");
        }
        assert(right >= _a);
        assert(left <= _b + 1);
        left = std::max(left, _a);
        right = std::min(right, _b + 1);
        assert(right > left);
        std::vector<double> new_probs(_p.begin() + (left - _a), _p.begin() + (right - _a));
        std::for_each(new_probs.begin(), new_probs.end(), [p_ab](double& np){ np = std::min(1.0, std::max(0.0, np / p_ab)); });
        return new DiscreteDistribution(left, std::move(new_probs));
    }

	void DiscreteDistribution::assign_proba(const std::vector<double>& p) {
		check_equals(_p.size(), p.size());
		_p = p;
		calc_cum_p();
	}
}
