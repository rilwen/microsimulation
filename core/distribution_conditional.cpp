// (C) Averisera Ltd 2014-2020
#include "distribution_conditional.hpp"
#include <stdexcept>

namespace averisera {
    DistributionConditional::DistributionConditional(std::shared_ptr<const Distribution> orig, double a, double b, double mean, double variance)
        : _orig(orig), _a(a), _b(b), _mean(mean), _variance(variance) {
        if (!orig) {
            throw std::domain_error("DistributionConditional: null original distribution");
        }
        if (b <= a) {
            throw std::domain_error("DistributionConditional: b <= a");
        }
        _cdf2_a = orig->cdf2(a);
        _cdf2_b = orig->cdf2(b);
        _p_ab = orig->range_prob2(a, b);
        if (_p_ab <= 0) {
            throw std::runtime_error("DistributionConditional: P(X in [a, b)) <= 0");
        }
        validate_moments();
    }

    DistributionConditional::DistributionConditional(std::shared_ptr<const Distribution> orig, double a, double b)
        : DistributionConditional(orig, a, b, 0.5 * a + 0.5 * b, 0) {
        // Now we know that orig is not null
        // Calculate correct values for mean and variance
        _mean = orig->conditional_mean(a, b);
        _variance = orig->conditional_variance(_mean, a, b);
        validate_moments();
    }

    double DistributionConditional::pdf(double x) const {
        if (x < _a || x >= _b) {
            return 0;
        } else {
            return _orig->pdf(x) / _p_ab;
        }
    }

    double DistributionConditional::cdf(double x) const {
        // Calculate P(X <= x | X \in [a, b)) = P(X <= x && X \in [a, b)) / P(X \in [a, b))
        // For x < a, P(X <= x && X \in [a, b)) = 0
        // For x >= b, P(X <= x && X \in [a, b)) = P(X \in [a, b))
        // For x \in [a, b), P(X <= x && X \in [a, b)) = P(X \in [a, x]) = P(X <= x) - P(X < a)
        if (x < _a) {
            return 0;
        } else if (x >= _b) {
            return 1;
        } else {
            return (_orig->cdf(x) - _cdf2_a) / _p_ab;
        }
    }

    double DistributionConditional::cdf2(double x) const {
        // Calculate P(X < x | X \in [a, b)) = P(X < x && X \in [a, b)) / P(X \in [a, b))
        // For x <= a, P(X < x && X \in [a, b)) = 0
        // For x >= b, P(X < x && X \in [a, b)) = P(X \in [a, b))
        // For x \in (a, b), P(X < x && X \in [a, b)) = P(X \in [a, x)) = P(X < x) - P(X < a)
        if (x <= _a) {
            return 0;
        } else if (x >= _b) {
            return 1;
        } else {
            return (_orig->cdf2(x) - _cdf2_a) / _p_ab;
        }
    }

    double DistributionConditional::range_prob(double x1, double x2) const {
        // Calculate P(X \in (x1, x2] | X \in [a, b)) = P(X \in (x1, x2] && X \in [a, b)) / P(X \in [a, b))
        if (x1 >= _a) {
            if (x2 < _b) {
                // P(X \in (x1, x2] && X \in [a, b)) = P(X \in (x1, x2])
                return _orig->range_prob(x1, x2) / _p_ab;
            } else {
                // P(X \in (x1, x2] && X \in [a, b)) = P(X \in (x1, b))
                return (_cdf2_b - _orig->cdf(x1)) / _p_ab;
            }
        } else {
            if (x2 < _b) {
                // P(X \in (x1, x2] && X \in [a, b)) = P(X \in [a, x2])
                return (_orig->cdf(x2) - _cdf2_a) / _p_ab;
            } else {
                // P(X \in (x1, x2] && X \in [a, b)) = P(X \in [a,b))
                return 1;
            }
        }
    }

    double DistributionConditional::range_prob2(double x1, double x2) const {
        // Calculate P(X \in [x1, x2) | X \in [a, b)) = P(X \in [x1, x2) && X \in [a, b)) / P(X \in [a, b))
        // = P(X \in [max(x1, a), min(x2, b))) / P(X \in [a, b))
        return _orig->range_prob2(std::max(x1, _a), std::min(x2, _b)) / _p_ab;
    }

    double DistributionConditional::icdf(double p) const {
        p *= _p_ab;
        p += _cdf2_a;
		assert(p >= 0.0);
		assert(p <= 1.0);
        return _orig->icdf(p);
    }

    void DistributionConditional::validate_moments() const {
        if (_variance < 0) {
            throw std::domain_error("DistributionConditional: negative variance");
        }        
        if (_mean < _a || _mean >= _b) {
            throw std::domain_error("DistributionConditional: mean outside [a, b)");
        }
    }

    std::unique_ptr<Distribution> DistributionConditional::clone() const {
        return std::unique_ptr<Distribution>(new DistributionConditional(_orig, _a, _b, _mean, _variance));
    }
}
