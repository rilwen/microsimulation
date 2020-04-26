// (C) Averisera Ltd 2014-2020
#include "multivariate_distribution_gaussian.hpp"
#include "moore_penrose.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    MultivariateDistributionGaussian::MultivariateDistributionGaussian(const Eigen::VectorXd& mean, const Eigen::MatrixXd& covariance)
        : MultivariateDistributionGaussianSimple(mean, covariance), _covariance(covariance) {
    }

    void MultivariateDistributionGaussian::conditional(Eigen::Ref<const Eigen::VectorXd> a, Eigen::Ref<Eigen::VectorXd> new_mean, Eigen::Ref<Eigen::MatrixXd> new_covariance) const {
        conditional(mean(), _covariance, a, new_mean, new_covariance);
    }
    
    void MultivariateDistributionGaussian::conditional(Eigen::Ref<const Eigen::VectorXd> mean, Eigen::Ref<const Eigen::MatrixXd> covariance, Eigen::Ref<const Eigen::VectorXd> a, Eigen::Ref<Eigen::VectorXd> new_mean, Eigen::Ref<Eigen::MatrixXd> new_covariance) {
        if (mean.size() != covariance.rows() || covariance.rows() != covariance.cols() || mean.size() != a.size()) {
            throw std::domain_error("MultivariateDistributionGaussian: size mismatch in input");
        }
        const unsigned int dim = static_cast<unsigned int>(mean.size());
        const unsigned int nbr_free = static_cast<unsigned int>(new_mean.size());
        if (nbr_free > dim) {
            throw std::domain_error("MultivariateDistributionGaussian: too large output matrices");
        }
        if (new_mean.size() != new_covariance.rows() || new_covariance.rows() != new_covariance.cols()) {
            throw std::domain_error("MultivariateDistributionGaussian: size mismatch in output");
        }
        const unsigned int nbr_specified = dim - nbr_free;
        // find out which factors have been specified
        std::vector<unsigned int> free_factors;
        std::vector<unsigned int> specified_factors;
        specified_factors.reserve(nbr_specified);
        free_factors.reserve((nbr_free));        
        for (unsigned int i = 0; i < dim; ++i) {
            if (!std::isnan(a[i])) {
                specified_factors.push_back(i);
            } else {
                free_factors.push_back(i);
            }
        }
        if (nbr_free != free_factors.size()) {
            throw std::domain_error(boost::str(boost::format("MultivariateDistributionGaussian: bad number of free variables: got %d instead of %d") % free_factors.size() % nbr_free));
        }
        if (nbr_specified != specified_factors.size()) {
            throw std::domain_error(boost::str(boost::format("MultivariateDistributionGaussian: bad number of specified variables: got %d instead of %d") % specified_factors.size() % nbr_specified));
        }

        // rearrange matrix and vector elements
        Eigen::MatrixXd G22inv(nbr_specified, nbr_specified);
        Eigen::MatrixXd G12(nbr_free, nbr_specified);
        Eigen::MatrixXd regression_coefficients(nbr_free, nbr_specified);
        Eigen::VectorXd v(nbr_specified);
        for (unsigned int i = 0; i < nbr_free; ++i) {
            const unsigned int k_i = free_factors[i];
            for (unsigned int j = 0; j < nbr_specified; ++j) {
                G12(i, j) = covariance(k_i, specified_factors[j]);
            }
        }
        for (unsigned int i = 0; i < nbr_specified; ++i) {
            const unsigned int k_i = specified_factors[i];
            v[i] = a[k_i] - mean[k_i];
            G22inv(i, i) = covariance(k_i, k_i);
            for (unsigned int j = 0; j < i; ++j) {
                G22inv(i, j) = G22inv(j, i) = covariance(k_i, specified_factors[j]);
            }
        }
        // calculate G22^{-1}
        MoorePenrose::inverse(G22inv, 0, G22inv);
        // calculate G12 * G22^{-1}, a.k.a. regression coefficients matrix
        regression_coefficients.noalias() = G12 * G22inv;
        // new_cov = G11 - G12 * G22^{-1} * G21
        // calculate - G12 * G22^{-1} * G21
        new_covariance.noalias() = regression_coefficients * G12.transpose();
        new_covariance *= -1;
        new_mean = regression_coefficients * v;
        // add G11
        for (unsigned int i = 0; i < nbr_free; ++i) {
            const unsigned int k_i = free_factors[i];
            new_mean[i] += mean[k_i];
            new_covariance(i, i) = new_covariance(i, i) + covariance(k_i, k_i);
            for (unsigned int j = 0; j < i; ++j) {
                const double cov_ij = covariance(k_i, free_factors[j]);
                new_covariance(i, j) += cov_ij;
                new_covariance(j, i) += cov_ij;
            }
        }
    }
}
