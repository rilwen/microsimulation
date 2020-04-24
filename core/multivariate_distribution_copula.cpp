#include "multivariate_distribution_copula.hpp"
#include "copula.hpp"
#include "distribution.hpp"
#include <algorithm>
#include <stdexcept>

namespace averisera {
    MultivariateDistributionCopula::MultivariateDistributionCopula(std::shared_ptr<const Copula> copula, const std::vector<std::shared_ptr<const Distribution>>& marginals)
    : _copula(copula), _marginals(marginals) {
        if (!_copula) {
            throw std::domain_error("MultivariateDistributionCopula: null copula");
        }
        if (_copula->dim() != _marginals.size()) {
            throw std::domain_error("MultivariateDistributionCopula: bad maginals size");
        }
        if (std::any_of(_marginals.begin(), _marginals.end(), [](const std::shared_ptr<const Distribution>& ptr){ return !ptr; })) {
            throw std::domain_error("MultivariateDistributionCopula: null marginal distribution");
        }
    }
        
    size_t MultivariateDistributionCopula::dim() const {
        return _copula->dim();
    }
        
    void MultivariateDistributionCopula::draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        _copula->draw_cdfs(rng, x);
        marginal_icdf(x, x);
    }
        
    void MultivariateDistributionCopula::marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const {
        const auto d = dim();
        if (static_cast<unsigned int>(x.size()) != d || static_cast<unsigned int>(p.size()) != d) {
            throw std::domain_error("MultivariateDistributionCopula: size mismatch");
        }
        std::transform(x.data(), x.data() + dim(), _marginals.begin(), p.data(), [](double v, const std::shared_ptr<const Distribution>& d){ return d->cdf(v); });
    }
        
    void MultivariateDistributionCopula::marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const {
        const auto d = dim();
        if (static_cast<unsigned int>(x.size()) != d || static_cast<unsigned int>(p.size()) != d) {
            throw std::domain_error("MultivariateDistributionCopula: size mismatch");
        }
        std::transform(p.data(), p.data() + dim(), _marginals.begin(), x.data(), [](double v, const std::shared_ptr<const Distribution>& d){ return d->icdf(v); });
    }
    
    void MultivariateDistributionCopula::adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const {
        const auto d = dim();
        if (static_cast<unsigned int>(sample.cols()) != d) {
            throw std::domain_error("MultivariateDistributionCopula: dimension mismatch");
        }
        typedef decltype(sample.rows()) index_t;
        for (unsigned int c = 0; c < d; ++c) {        
            auto col = sample.col(c);
            const auto marginal = _marginals[c];
            for (index_t r = 0; r < sample.rows(); ++r) {
                col[r] = marginal->cdf(col[r]);
            }
        }
        _copula->adjust_cdfs(sample);
        for (unsigned int c = 0; c < d; ++c) {        
            auto col = sample.col(c);
            const auto marginal = _marginals[c];            
            for (index_t r = 0; r < sample.rows(); ++r) {
                col[r] = marginal->icdf(col[r]);
            }
        }
    }
}
