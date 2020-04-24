#include "copula_multifactor.hpp"
#include <algorithm>
#include <stdexcept>

namespace averisera {
    CopulaMultifactor::CopulaMultifactor(size_t dim)
    : _dim(dim) {
    }

	CopulaMultifactor::CopulaMultifactor(CopulaMultifactor&& other)
		: _dim(other._dim) {
		other._dim = 0;
	}
    
    void CopulaMultifactor::draw_cdfs(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        if (static_cast<unsigned int>(x.size()) != _dim) {
            throw std::domain_error("CopulaMultifactor: bad vector size");
        }
        draw_corr_factors(rng, x);
        std::transform(x.data(), x.data() + _dim, x.data(), [this](double v) { return marginal_factor_cdf(v); });
    }
}
