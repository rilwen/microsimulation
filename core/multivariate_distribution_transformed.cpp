#include "multivariate_distribution_transformed.hpp"
#include <algorithm>

namespace averisera {
    MultivariateDistributionTransformed::MultivariateDistributionTransformed(const MultivariateDistribution* orig, const std::vector<std::function<double(double)>>& transforms, const std::vector<std::function<double(double)>>& inverse_transforms)
        : _orig(orig), _transforms(transforms), _inverse_transforms(inverse_transforms) {
        validate();
    }

    MultivariateDistributionTransformed::MultivariateDistributionTransformed(const MultivariateDistribution* orig, std::vector<std::function<double(double)>>&& transforms, std::vector<std::function<double(double)>>&& inverse_transforms)
        : _orig(orig), _transforms(std::move(transforms)), _inverse_transforms(std::move(inverse_transforms)) {
        validate();
    }
    
    void MultivariateDistributionTransformed::draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        _orig->draw(rng, x);
        std::transform(x.data(), x.data() + dim(), _transforms.begin(), x.data(), [](double v, std::function<double(double)> f){ return f(v); });
    }

    void MultivariateDistributionTransformed::draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const {
        _orig->draw_noncont(rng, x);
        const size_t n = dim();
        auto trit = _transforms.begin();
        for (size_t i = 0; i < n; ++i) {
            x[i] = (*trit)(x[i]);
            ++trit;
        }
    }
        
    void MultivariateDistributionTransformed::marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const {
        const size_t d = dim();
        std::transform(x.data(), x.data() + d, _inverse_transforms.begin(), p.data(), [](double v, std::function<double(double)> f){ return f(v); });
        _orig->marginal_cdf(p, p);
    }
        
    void MultivariateDistributionTransformed::marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const {        
        _orig->marginal_icdf(p, x);
        const size_t d = dim();
        std::transform(x.data(), x.data() + d, _transforms.begin(), x.data(), [](double v, std::function<double(double)> f){ return f(v); });
    }

    void MultivariateDistributionTransformed::adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const {
        const auto d = dim();
        if (static_cast<size_t>(sample.cols()) != d) {
            throw std::domain_error("MultivariateDistributionTransformed: dimension mismatch");
        }
        typedef decltype(sample.rows()) index_t;
        for (index_t r = 0; r < sample.rows(); ++r) {
            auto pt = sample.row(r);
            for (size_t c = 0; c < d; ++c) {
                pt[c] = _inverse_transforms[c](pt[c]);
            }
        }
        _orig->adjust_distribution(sample);
        for (index_t r = 0; r < sample.rows(); ++r) {
            auto pt = sample.row(r);
            for (size_t c = 0; c < d; ++c) {
                pt[c] = _transforms[c](pt[c]);
            }
        }
    }

    void MultivariateDistributionTransformed::validate() const {
        if (!_orig) {
            throw std::domain_error("MultivariateDistributionTransformed: original distribution is null");
        }
        if (_transforms.size() != _orig->dim() || _inverse_transforms.size() != _orig->dim()) {
            throw std::domain_error("MultivariateDistributionTransformed: vector size mismatch");
        }
    }
}
