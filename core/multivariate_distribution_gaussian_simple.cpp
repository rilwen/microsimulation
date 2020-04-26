// (C) Averisera Ltd 2014-2020
#include "multivariate_distribution_gaussian_simple.hpp"
#include "rng.hpp"
#include "normal_distribution.hpp"
#include "statistics.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <Eigen/SVD>

namespace averisera {
    MultivariateDistributionGaussianSimple::MultivariateDistributionGaussianSimple(const Eigen::VectorXd& mean, const Eigen::MatrixXd& covariance)
        : _mean(mean), _sigmas(mean.size()) {
        if (covariance.rows() != covariance.cols()) {
            throw std::domain_error("MultivariateDistributionGaussian: covariance matrix is not square");
        }
        if (mean.size() != covariance.rows()) {
            throw std::domain_error("MultivariateDistributionGaussian: covariance and mean size mismatch");
        }
        std::string errmsg;
        if (!Statistics::check_covariance_matrix(covariance, &errmsg)) {
            throw std::domain_error(std::string("MultivariateDistributionGaussian: bad covariance matrix: ") + errmsg);
        }
        Eigen::JacobiSVD<Eigen::MatrixXd> svd(covariance, Eigen::ComputeThinU | Eigen::ComputeThinV);
        _S = svd.matrixU().block(0, 0, covariance.rows(), covariance.rows());
        _invS = _S.transpose();
        for (size_t i = 0; i < dim(); ++i) {
            _sigmas[i] = sqrt(covariance(i, i));
            double lambda = svd.singularValues()[i];
            if (lambda < -1E12) {
                throw std::runtime_error("MultivariateDistributionGaussian:: rho is not positive semidefinite");
            }
            lambda = sqrt(std::max(lambda, 0.));
            _S.col(i) *= lambda;
            if (lambda > 0) {
                _invS.row(i) /= lambda;
            } else {
                _invS.row(i).setZero();
            }
        }
    }
    
    void MultivariateDistributionGaussianSimple::draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        rng.next_gaussians(_S, x);
        std::transform(x.data(), x.data() + dim(), _mean.data(), x.data(), [](double v, double mu) { return v + mu; });
    }

    void MultivariateDistributionGaussianSimple::draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const {
        rng.next_gaussians_noncont(_S, x);
        const size_t n = dim();
        const double* mu = _mean.data();
        for (size_t i = 0; i < n; ++i) {
            x[i] += *mu;
            ++mu;
        }
    }
    
    void MultivariateDistributionGaussianSimple::marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const {
        const auto d = dim();
        if (static_cast<size_t>(x.size()) != d || static_cast<size_t>(p.size()) != d) {
            throw std::domain_error("MultivariateDistributionGaussianSimple: size mismatch");
        }
        std::transform(x.data(), x.data() + d, _mean.data(), p.data(), [](double v, double mu) { return v - mu; });
        std::transform(p.data(), p.data() + d, _sigmas.data(), p.data(), [](double v, double sigma) { return NormalDistribution::normcdf( v / sigma ); });
    }
        
    void MultivariateDistributionGaussianSimple::marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const {
        const auto d = dim();
        if (static_cast<size_t>(x.size()) != d || static_cast<size_t>(p.size()) != d) {
            throw std::domain_error("MultivariateDistributionGaussianSimple: size mismatch");
        }
        std::transform(p.data(), p.data() + d, _sigmas.data(), x.data(), [](double p, double sigma) { return sigma * NormalDistribution::normsinv( p ); });
        std::transform(x.data(), x.data() + d, _mean.data(), x.data(), [](double v, double mu) { return v + mu; });
    }

    void MultivariateDistributionGaussianSimple::adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const {
        const auto d = dim();
        if (static_cast<size_t>(sample.cols()) != d) {
            throw std::domain_error("MultivariateDistributionGaussianSimple: dimension mismatch");
        }
        const auto nr = sample.rows();
        const auto nc = sample.cols();
        typedef decltype(sample.rows()) index_t;
        for (index_t c = 0; c < nc; ++c) {
            sample.col(c) -= Eigen::VectorXd::Constant(nr, _mean[c]);
        }
        Eigen::MatrixXd iid_sample(sample.rows(), sample.cols());
        // (iid_sample)^T = invS * sample^T
        // iid_sample = sample * (invS)^T
        iid_sample.noalias() = sample * _invS.transpose();
        Statistics::percentiles_inplace(iid_sample);
        for (index_t c = 0; c < nc; ++c) {
            auto col = iid_sample.col(c);
            for (index_t r = 0; r < nr; ++r) {
                col[r] = NormalDistribution::normsinv(col[r]);
            }
        }
        sample.noalias() = iid_sample * _S.transpose();
        for (index_t c = 0; c < nc; ++c) {
            sample.col(c) += Eigen::VectorXd::Constant(nr, _mean[c]);            
        }
    }
}

