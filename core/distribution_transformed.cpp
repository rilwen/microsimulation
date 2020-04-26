// (C) Averisera Ltd 2014-2020
#include "distribution_transformed.hpp"
#include "preconditions.hpp"
#include <stdexcept>

namespace averisera {
    DistributionTransformed::DistributionTransformed(const Distribution* orig, std::function<double(double)> transform, std::function<double(double)> transform_derivative, std::function<double(double)> inverse_transform)
    : _orig(orig), _transform(transform), _transform_derivative(transform_derivative), _inverse_transform(inverse_transform) {
		check_that(orig != nullptr, "DistributionTransformed: null original distribution");
    }
    
    double DistributionTransformed::pdf(double x) const {
        // Y = F(X)
        // dY = dF/dX * dX
        // rho_X dX = rho_Y dY = rho_Y dF/dX dX
        // rho_X = dF/dX rho_Y
        // rho_Y = rho_X / (dF/dX)
        return _orig->pdf(_inverse_transform(x)) / _transform_derivative(x);
    }
            
    double DistributionTransformed::cdf(double x) const {
        return _orig->cdf(_inverse_transform(x));
    }

	double DistributionTransformed::cdf2(double x) const {
		return _orig->cdf2(_inverse_transform(x));
	}
            
    double DistributionTransformed::range_prob(double x1, double x2) const {
        return _orig->range_prob(_inverse_transform(x1), _inverse_transform(x2));
    }
            
    double DistributionTransformed::icdf(double p) const {
		return _transform(_orig->icdf(p));
    }

	double DistributionTransformed::infimum() const {
		return _transform(_orig->infimum());
	}

	double DistributionTransformed::supremum() const {
		return _transform(_orig->supremum());
	}
}
