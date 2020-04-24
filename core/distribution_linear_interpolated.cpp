#include "distribution_linear_interpolated.hpp"
#include <stdexcept>
#include <cmath>
#include <cassert>
//#include <iostream>

namespace averisera {
    namespace {
        static double calculate_mean(const std::vector<double>& x, const std::vector<double>& p) {
            const size_t n = p.size();
            double mean = 0.;
            for (size_t i = 0; i < n; ++i) {
                mean += p[i] * (x[i] + (x[i + 1] - x[i]) / 2);
            }
            return mean;
        }

        static double calculate_variance(const std::vector<double>& x, const std::vector<double>& p, const double mean) {
            /*
              int_a^b (x - m)^2 = (x - m)^3 / 3 |_a^b = ((b - m)^3 - (a - m)^3) / 3
            */
            const size_t n = p.size();
            double var = 0.;
            for (size_t i = 0; i < n; ++i) {
                const double am = x[i] - mean;
                const double bm = x[i + 1] - mean;
                var += p[i] * (pow(bm, 3) - pow(am, 3)) / 3;
            }
            return var;
        }
    }

    DistributionLinearInterpolated::DistributionLinearInterpolated(std::vector<double>&& x, std::vector<double>&& p) {
        validate(x, p);
		_x = std::move(x);
		_p = std::move(p);
        _cp.resize(_p.size());
        calculate_cumulative_proba(_p, _cp);
        _mean = calculate_mean(_x, _p);
        _variance = calculate_variance(_x, _p, _mean);
    }

    DistributionLinearInterpolated::DistributionLinearInterpolated(DistributionLinearInterpolated&& other)
        : _x(std::move(other._x)), _p(std::move(other._p)), _cp(std::move(other._cp)), _mean(other._mean), _variance(other._variance) {        
    }

    DistributionLinearInterpolated::DistributionLinearInterpolated(const DistributionLinearInterpolated& other)
        : _x(other._x), _p(other._p), _cp(other._cp), _mean(other._mean), _variance(other._variance) {        
    }

    double DistributionLinearInterpolated::pdf(double x) const {
        if (x < _x.front() || x > _x.back()) {
            return 0.;
        }
        size_t i = idx(_x, x);
        if (i > 0) {
            --i;
        }
        return _p[i] / (_x[i + 1] - _x[i]);
    }

    double DistributionLinearInterpolated::cdf(const double x) const {
        if (x <= _x.front()) {
            return 0.;
        } else if (x >= _x.back()) {
            return 1.;
        } else {
            const size_t i = idx(_x, x);
            assert(i > 0);
            const double x1 = _x[i - 1];
            const double x2 = _x[i];
            assert(x >= x1);
            assert(x <= x2);
            assert(x2 > x1);
            const double p2 = _cp[i - 1]; // _cp[i] = P(x <= x[i + 1])
            const double p1 = i > 1 ? _cp[i - 2] : 0.;
            assert(p2 >= p1);
            return (p1 * (x2 - x) + p2 * (x - x1)) / (x2 - x1);
        }
    }

    double DistributionLinearInterpolated::icdf(const double p) const {
        if (p < 0 || p > 1) {
            throw std::out_of_range("DistributionLinearInterpolated: probability outside [0, 1]");
        }
        if (p == 0) {
            return _x.front();
        }
        if (p == 1) {
            return _x.back();
        }
        const size_t i = idx(_cp, p);
        assert(i < _cp.size());
        const double x1 = _x[i];
        const double x2 = _x[i + 1];
        const double p2 = _cp[i];
        const double p1 = i > 0 ? _cp[i - 1] : 0.;
        assert(p >= p1);
        assert(p <= p2);
        //std::cerr << p1 << " " << p2 << ", " << x1 << " " << x2 << std::endl;
        return (x1 * (p2 - p) + x2 * (p - p1)) / (p2 - p1);
    }

    std::unique_ptr<Distribution> DistributionLinearInterpolated::clone() const {
        return std::unique_ptr<Distribution>(new DistributionLinearInterpolated(*this));
    }

	bool DistributionLinearInterpolated::operator==(const DistributionLinearInterpolated& other) const {
		if (this != &other) {
			return _x == other._x && _p == other._p;
		} else {
			return false;
		}
	}
}
