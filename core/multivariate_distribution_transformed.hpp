#ifndef __AVERISERA_MULTIVARIATE_DISTRIBUTION_TRANSFORMED_H
#define __AVERISERA_MULTIVARIATE_DISTRIBUTION_TRANSFORMED_H

#include "multivariate_distribution.hpp"
#include <functional>
#include <memory>
#include <vector>

namespace averisera {
    /** Distribution of Y_i = F_i(X_i) */
    class MultivariateDistributionTransformed: public MultivariateDistribution {
    public:
        /** @param orig Pointer to distribution of X (constructed object takes ownership)
         * @param transforms Vector of transformations F_i (strictly monotonically increasing functions)
         * @param inverse_transforms Vector of inverse transformations F_i^{-1} (strictly monotonically increasing functions)
         * @throw std::domain_error If orig is null or transforms vectors sizes do not match orig->dim()
         */
        MultivariateDistributionTransformed(const MultivariateDistribution* orig, const std::vector<std::function<double(double)>>& transforms, const std::vector<std::function<double(double)>>& inverse_transforms);

        /** Move constructor */
        MultivariateDistributionTransformed(const MultivariateDistribution* orig, std::vector<std::function<double(double)>>&& transforms, std::vector<std::function<double(double)>>&& inverse_transforms);
        
        size_t dim() const override {
            return _orig->dim();
        }

        using MultivariateDistribution::draw; // so that we get non-virtual draw as well
        
        void draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;

        void draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const override;

        void marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const override;

        void marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const override;

        void adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const override;
    private:
        void validate() const;
        
        std::unique_ptr<const MultivariateDistribution> _orig;
        std::vector<std::function<double(double)>> _transforms;
        std::vector<std::function<double(double)>> _inverse_transforms;
    };
}

#endif // __AVERISERA_MULTIVARIATE_DISTRIBUTION_TRANSFORMED_H
