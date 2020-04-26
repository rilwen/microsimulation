// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MULTIVARIATE_DISTRIBUTION_GAUSSIAN_H
#define __AVERISERA_MULTIVARIATE_DISTRIBUTION_GAUSSIAN_H

#include "multivariate_distribution_gaussian_simple.hpp"

namespace averisera {
    /** Multivariate Gaussian distribution - rich version with covariance data */
    class MultivariateDistributionGaussian: public MultivariateDistributionGaussianSimple {
    public:
        /** @see MultivariateDistributionGaussianSimple
         */
        MultivariateDistributionGaussian(const Eigen::VectorXd& mean, const Eigen::MatrixXd& covariance);

        MultivariateDistributionGaussian(const MultivariateDistributionGaussian& other) = default;
        
        using MultivariateDistribution::draw; // so that we get non-virtual draw as well

        /** Calculate the parameters a distribution conditioned on some of the correlated Gaussian variables being set
          @param[in] a Vector with specified variables, of size dim(). Free variables should be set to NaN.
          @param[out] new_mean Mean of the conditional distribution. Must have size equal to number of free variables.
          @param[out] new_covariance Mean of the conditional distribution. Must have dimension equal to number of free variables.
          @throw std::domain_error If a.size() != dim() or any other dimensions/sizes do not match, or bad number of free/specified variables.
        */
        void conditional(Eigen::Ref<const Eigen::VectorXd> a, Eigen::Ref<Eigen::VectorXd> new_mean, Eigen::Ref<Eigen::MatrixXd> new_covariance) const;

        /** Static version of conditional(Eigen::Ref<const Eigen::VectorXd> a, Eigen::Ref<Eigen::VectorXd> new_mean, Eigen::Ref<Eigen::MatrixXd> new_covariance).
          @throws If mean.size() != covariance.rows() or covariance.cols() != covariance.rows()
         */
        static void conditional(Eigen::Ref<const Eigen::VectorXd> mean, Eigen::Ref<const Eigen::MatrixXd> covariance, Eigen::Ref<const Eigen::VectorXd> a, Eigen::Ref<Eigen::VectorXd> new_mean, Eigen::Ref<Eigen::MatrixXd> new_covariance);
    private:
        Eigen::MatrixXd _covariance;
    };
}

#endif // __AVERISERA_MULTIVARIATE_DISTRIBUTION_GAUSSIAN_H
