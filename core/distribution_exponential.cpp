#include "distribution_exponential.hpp"
#include "preconditions.hpp"
#include <stdexcept>

namespace averisera {
    DistributionExponential::DistributionExponential(double lambda_neg, double lambda_pos)
    : _lambda_neg(lambda_neg), _lambda_pos(lambda_pos) {
        if (lambda_neg <= 0 || lambda_pos <= 0) {
            throw std::out_of_range("DistributionExponential: lambdas must be positive");
        }
        _cdf0 = 1 / _lambda_neg;
        _norm = _cdf0 + 1 / _lambda_pos;
        _cdf0 /= _norm;
        // int_0^infty exp(-l*x) x dx = 1 / l^2        
        _mean = (1 / (_lambda_pos * _lambda_pos) - 1 / (_lambda_neg * _lambda_neg)) / _norm;
        // int_0^infty exp(-l*x) x^2 dx = 2 / l^3
        // Var(X) = E[X^2] - (E[X])^2
        _variance = std::max(0.0, 2 * (1 / (_lambda_pos * _lambda_pos * _lambda_pos) + 1 / (_lambda_neg * _lambda_neg * _lambda_neg)) / _norm - _mean * _mean);
    }
    
    double DistributionExponential::cdf(double x) const {
        if (x < 0) {
            return exp(_lambda_neg * x) / (_lambda_neg * _norm);
        } else {
            return _cdf0 - expm1(- _lambda_pos * x) / (_lambda_pos * _norm);
        }
    }
        
    double DistributionExponential::icdf(double p) const {
		check_that<std::out_of_range>(p >= 0, "DistributionExponential: Probability less than 0");
		check_that<std::out_of_range>(p <= 1, "DistributionExponential: Probability more than 1");
        if (p < _cdf0) {
            // p == exp(_lambda_neg * x) / (_lambda_neg * _norm)
            return log(p * _lambda_neg * _norm) / _lambda_neg;
        } else {
            const double dp = p - _cdf0;
            // p == cdf0 - expm1(- _lambda_pos * x) / (_lambda_pos * _norm)
            // - dp == expm1(- _lambda_pos * x) / (_lambda_pos * _norm)
            // 1 - _lambda_pos * _norm * dp == exp(- _lambda_pos * x)
            // log(1 - _lambda_pos * _norm * dp) == - _lambda_pos * x
            // log1p(- _lambda_pos * _norm * dp) == - _lambda_pos * x
            return - log1p(- dp * _lambda_pos * _norm) / _lambda_pos;
        }
    }

	double DistributionExponential::infimum() const {
		if (std::isfinite(_lambda_neg)) {
			return -std::numeric_limits<double>::infinity();
		} else {
			return 0.0;
		}
	}

	double DistributionExponential::supremum() const {
		if (std::isfinite(_lambda_pos)) {
			return std::numeric_limits<double>::infinity();
		} else {
			return 0.0;
		}
	}
}
