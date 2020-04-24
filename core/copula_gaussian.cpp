#include "copula_gaussian.hpp"
#include "eigen.hpp"
#include "multivariate_distribution_gaussian.hpp"
#include "normal_distribution.hpp"
#include "statistics.hpp"
#include <algorithm>
#include <cmath>


namespace averisera {
    CopulaGaussian::CopulaGaussian(const Eigen::MatrixXd& rho, double min_variance_fract, unsigned int max_nbr_factors)
        : CopulaAlphaStable(rho, min_variance_fract, max_nbr_factors), _rho(rho) {
    }

	CopulaGaussian::CopulaGaussian(CopulaGaussian&& other)
		: CopulaAlphaStable(std::move(other)),
		_rho(std::move(other._rho)) {}

    MultivariateDistributionGaussianSimple* CopulaGaussian::make_conditional(Eigen::Ref<const Eigen::VectorXd> a, unsigned int cond_dim) const {
        if (static_cast<unsigned int>(a.size()) != dim()) {
            throw std::domain_error("CopulaGaussian: size mismatch");
        }
        if (cond_dim > dim()) {
            throw std::domain_error("CopulaGaussian: conditional dimension too large");
        }
        Eigen::MatrixXd cond_cov;
        Eigen::VectorXd cond_mean;                
        cond_mean.resize(cond_dim);
        cond_cov.resize(cond_dim, cond_dim);
        MultivariateDistributionGaussian::conditional(Eigen::VectorXd::Zero(dim()), _rho, a, cond_mean, cond_cov);
        assert(cond_mean.size() <= _rho.rows());
        return new MultivariateDistributionGaussianSimple(cond_mean, cond_cov); // use simple version to avoid copying covariance matrix
    }

	CopulaGaussian CopulaGaussian::from_sample(const Eigen::MatrixXd& sample, double min_variance_fract, unsigned int max_nbr_factors) {
		const size_t nr = static_cast<size_t>(sample.rows());
		const size_t nc = static_cast<size_t>(sample.cols());
		Eigen::MatrixXd work(sample);
		Statistics::percentiles_inplace(work);
		for (size_t c = 0; c < nc; ++c) {
			const auto b = work.col(c).data();
			// col-wise layout
			std::transform(b, b + nr, b, [](const double& p) { return NormalDistribution::normsinv(p); });
		}
		Eigen::MatrixXd corr;
		Statistics::estimate_covariance_matrix(work, DataCheckLevel::FINITE, corr);
		for (size_t c1 = 0; c1 < nc; ++c1) {
			const double sigma1 = sqrt(std::max(0.0, corr(c1, c1)));
			for (size_t c2 = 0; c2 < nc; ++c2) {
				corr(c1, c2) /= sigma1;
				corr(c2, c1) /= sigma1;
			}
			corr(c1, c1) = 1.0;
		}
		return CopulaGaussian(corr, min_variance_fract, max_nbr_factors);
	}

	CopulaGaussian CopulaGaussian::from_sample(CSVFileReader& reader, const std::vector<size_t>& indices, double min_variance_fract, unsigned int max_nbr_factors) {
		return from_sample(EigenUtils::read_matrix(reader, indices, true), min_variance_fract, max_nbr_factors);
	}

	CopulaGaussian CopulaGaussian::from_sample(CSVFileReader& reader, const std::vector<std::string>& names, double min_variance_fract, unsigned int max_nbr_factors) {
		return from_sample(EigenUtils::read_matrix(reader, names, true), min_variance_fract, max_nbr_factors);
	}
}
