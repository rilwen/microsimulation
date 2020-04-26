// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MULTIVARIATE_DISTRIBUTION_INDEPENDENT_H
#define __AVERISERA_MULTIVARIATE_DISTRIBUTION_INDEPENDENT_H

#include "multivariate_distribution.hpp"
#include <initializer_list>
#include <memory>
#include <vector>

namespace averisera {
    /** Multivariate distribution composed of several independent (multi)variate distributions. */
    class MultivariateDistributionIndependent: public MultivariateDistribution {
    public:
        /** @throw std::domain_error If any of members is null */
        MultivariateDistributionIndependent(const std::vector<std::shared_ptr<const MultivariateDistribution>>& members);

        size_t dim() const override {
            return _dim;
        }
        
        using MultivariateDistribution::draw; // so that we get non-virtual draw as well

        void draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;

        void draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const override;

        void marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const override;

        void marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const override;

        void adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const override;
    private:        
        std::vector<std::shared_ptr<const MultivariateDistribution>> _members;
        const size_t _nbr_members;
        size_t _dim;
    };
}

#endif // __AVERISERA_MULTIVARIATE_DISTRIBUTION_INDEPENDENT_H
