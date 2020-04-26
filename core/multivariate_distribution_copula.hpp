// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MULTIVARIATE_DISTRIBUTION_COPULA_H
#define __AVERISERA_MULTIVARIATE_DISTRIBUTION_COPULA_H

#include "multivariate_distribution.hpp"
#include <memory>

namespace averisera {
    class Copula;
    class Distribution;
    
    /** Multivariate distribution built from a copula joining N marginal distributions */
    class MultivariateDistributionCopula: public MultivariateDistribution {
    public:
        /** @param[in] copula Pointer to copula implementation
         * @param[in] marginals Vector of pointers to marginal distributions
         * @throw std::domain_error If copula == nullptr, copula->dim() != marginals.size() or any marginals[i] == nullptr
         */
        MultivariateDistributionCopula(std::shared_ptr<const Copula> copula, const std::vector<std::shared_ptr<const Distribution>>& marginals);
        
        size_t dim() const override;

        using MultivariateDistribution::draw; // so that we get non-virtual draw as well
        
        void draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;

        void draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const override;

        void marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const override;

        void marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const override;

        void adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const override;
    private:
        std::shared_ptr<const Copula> _copula;
        std::vector<std::shared_ptr<const Distribution>> _marginals;
    };
}

#endif // __AVERISERA_MULTIVARIATE_DISTRIBUTION_COPULA_H
