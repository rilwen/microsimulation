/*
 * (C) Averisera Ltd 2015
 */

#include "rng_impl.hpp"
#include <cassert>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include "cauchy_distribution.hpp"
#include "math_utils.hpp"

namespace averisera {
    
    RNGImpl::RNGImpl(long seed)
    : _rng(seed), _u01(0., 1.), _n01(0., 1.) {
    }
    
    double RNGImpl::next_uniform() {
        return _u01(_rng);
    }
    
    double RNGImpl::next_gaussian() {
        return _n01(_rng);
    }

    uint64_t RNGImpl::next_uniform(uint64_t n) {
        std::uniform_int_distribution<uint64_t> u0n(0, n);
        return u0n(_rng);
    }
    
    double RNGImpl::next_alpha_stable(const double alpha) {
        if (alpha == 2) {
            return next_gaussian();
        } else if (alpha == 1) {
            // Cauchy
            return CauchyDistribution::icdf(next_uniform());
        } else {
            assert(alpha > 0);
            assert(alpha < 2);
            const double V = MathUtils::pi / 2 * (2 * next_uniform() - 1);
            const double W = -log(next_uniform());
            const double r = sin(alpha * V) / ( pow(cos(V), (1 / alpha)) ) * pow( cos( V * (1 - alpha) ) / W , (1 - alpha) / alpha );
            return r;
        }
    }
    
    template <class F, class V> void next_vector(const Eigen::MatrixXd& S, V y, F next_value) {
        const unsigned int n = static_cast<unsigned int>(y.size());
        if (n != static_cast<unsigned int>(S.rows())) {
            throw std::domain_error("RNGImpl: S and x dimensions do not match");
        }
        const unsigned int m = static_cast<unsigned int>(S.cols());
        Eigen::VectorXd x(m); // TODO: how to avoid repeated heap allocation?
        for (unsigned int i = 0; i < m; ++i) {
            x[i] = next_value();
        }
        //Eigen::Map<Eigen::VectorXd> ym(&y[0], n);
        y = S * x;
    }

    void RNGImpl::next_gaussians(const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd> y) {
        next_vector(S, y, [this](){ return next_gaussian(); });
    }

    void RNGImpl::next_gaussians_noncont(const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> y) {
        next_vector(S, y, [this](){ return next_gaussian(); });
    }
    
    void RNGImpl::next_alpha_stable(double alpha, const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd> y) {
        next_vector(S, y, [this, alpha](){ return next_alpha_stable(alpha); });
    }

	std::string RNGImpl::to_string() const {
		std::stringstream ss;
		ss << _rng;
		return ss.str();
	}
}
