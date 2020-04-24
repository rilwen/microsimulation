#ifndef __AVERISERA_MULTIVARIATE_DISTRIBUTION_DISCRETE_H
#define __AVERISERA_MULTIVARIATE_DISTRIBUTION_DISCRETE_H

#include "multivariate_distribution.hpp"
#include "discrete_distribution.hpp"
#include "multi_index_multisize.hpp"
#include <Eigen/Core>
#include <vector>

namespace averisera {
    /** Multivariate discrete distribution */
    class MultivariateDistributionDiscrete: public MultivariateDistribution {
    public:
        /** @param probs Vector with probabilities laid out with the last index changing fastest: [p_0000, p_0001, p_0002, ..., p_9998, p_9999]
          @param lb Lower bounds for each dimension (inclusive)
          @param ub Upper bounds for each dimension (inclusive)
          @throw std::domain_error If lb.size() != ub.size(), lb.size() == 0, p.size() != prod_i (ub[i] - lb[i] + 1) or p not a good probabilities vector.
        */
        MultivariateDistributionDiscrete(Eigen::Ref<const Eigen::VectorXd> probs, const std::vector<int>& lb, const std::vector<int>& ub);

        /** 2D case - from a probability matrix P_kl = P(X0 = k, X1 = l) */
        MultivariateDistributionDiscrete(Eigen::Ref<const Eigen::MatrixXd> probs, int lb0, int lb1);
        
        size_t dim() const override {
            return _mi.dim();
        }
        
        void draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;
        void draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const override;
        void marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const override;
        void marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const override;
        void adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const override;
    private:
        DiscreteDistribution _flat;
        std::vector<int> _lb;
        MultiIndexMultisize _mi;
        std::vector<DiscreteDistribution> _marginals; /**< Marginal distributions */
    };
}

#endif // __AVERISERA_MULTIVARIATE_DISTRIBUTION_DISCRETE_H
