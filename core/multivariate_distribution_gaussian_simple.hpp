// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MULTIVARIATE_DISTRIBUTION_GAUSSIAN_SIMPLE_H
#define __AVERISERA_MULTIVARIATE_DISTRIBUTION_GAUSSIAN_SIMPLE_H

#include "multivariate_distribution.hpp"
#include <Eigen/Core>

namespace averisera {
    /** Multivariate Gaussian distribution -- simple version which doesn't store covariance. */
    class MultivariateDistributionGaussianSimple: public MultivariateDistribution {
    public:
        /** @param mean Mean of the distribution
         * @param covariance Covariance of the distribution
         * @throw std::domain_error If mean.size() != covariance.rows() or covariance.rows() != covariance.cols().
         * @throw std::runtime_error If covariance is not positive semi-definite
         */
        MultivariateDistributionGaussianSimple(const Eigen::VectorXd& mean, const Eigen::MatrixXd& covariance);

        MultivariateDistributionGaussianSimple(const MultivariateDistributionGaussianSimple& other) = default;
        
        size_t dim() const override {
            return static_cast<size_t>(_sigmas.size());
        }

        const Eigen::MatrixXd& S() const {
            return _S;
        }

        const Eigen::MatrixXd& invS() const {
            return _invS;
        }

        using MultivariateDistribution::draw; // so that we get non-virtual draw as well
        
        void draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;

        void draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const override;

        void marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const override;

        void marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const override;

        void adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const override;
    protected:
        const Eigen::VectorXd& mean() const {
            return _mean;
        }
    private:
        Eigen::VectorXd _mean;
        Eigen::MatrixXd _S;
        Eigen::MatrixXd _invS;
        Eigen::VectorXd _sigmas;
    };
}

#endif // __AVERISERA_MULTIVARIATE_DISTRIBUTION_GAUSSIAN_SIMPLE_H
