#include "copula_alphastable.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include <Eigen/SVD>
#include "rng.hpp"
#include "math_utils.hpp"
#include "cauchy_distribution.hpp"
#include "normal_distribution.hpp"
#include "moore_penrose.hpp"
#include "statistics.hpp"

namespace averisera {
    
    static const double CASEPS = 1E-12;
    
    CopulaAlphaStable::CopulaAlphaStable(double alpha, const Eigen::MatrixXd& S)
    : CopulaMultifactor(static_cast<unsigned int>(S.rows())), _S(S), _alpha(alpha) {
        if (alpha <= 0 || alpha > 2) {
            throw std::out_of_range("CopulaAlphaStable: alpha out of range");
        }
        for (unsigned int i = 0; i < dim(); ++i) {
            const double scale = pow( _S.row(i).array().pow(_alpha).sum(), 1 / _alpha );
            _S.row(i) /= scale;
        }
        MoorePenrose::inverse(_S, CASEPS, _invS);
        assert(_invS.rows() == _S.cols());
        assert(_invS.cols() == _S.rows());
    }
    
    CopulaAlphaStable::CopulaAlphaStable(double alpha, const std::vector<double>& factor_loadings)
    : CopulaMultifactor(static_cast<unsigned int>(factor_loadings.size())), _alpha(alpha) {
        _S.setZero(dim(), dim() + 1);
        for (unsigned int i = 0; i < dim(); ++i) {
            const double beta = factor_loadings[i];
            if (std::abs(beta) > 1) {
                throw std::out_of_range("CopulaAlphaStable: beta outside [-1, 1]");
            }
            _S(i, i) = beta;
            _S(i, dim()) = pow(std::max(1 - pow(std::abs(beta), _alpha), 0.0), 1 / _alpha);
        }
        MoorePenrose::inverse(_S, CASEPS, _invS);
        assert(_invS.rows() == _S.cols());
        assert(_invS.cols() == _S.rows());
    }
    
    CopulaAlphaStable::CopulaAlphaStable(const Eigen::MatrixXd& rho, double min_variance_fract, unsigned int max_nbr_factors) 
    : CopulaMultifactor(static_cast<unsigned int>(rho.rows())), _alpha(2) {
        std::string errmsg;
        if (!Statistics::check_correlation_matrix(rho, &errmsg)) {
            throw std::domain_error(std::string("CopulaAlphaStable: bad correlation matrix: ") + errmsg);
        }
        if (min_variance_fract > 1.0) {
            throw std::domain_error("CopulaAlphaStable: minimum variance fraction above 1");
        }
        const unsigned int N = static_cast<unsigned int>(rho.rows());
        Eigen::JacobiSVD<Eigen::MatrixXd> svd(rho, Eigen::ComputeThinU | Eigen::ComputeThinV);
        unsigned int M = 0;
        double sumvar = 0;
        const unsigned int maxM = max_nbr_factors > 0 ? max_nbr_factors : N;
        const double maxvar = min_variance_fract * N;
        for (unsigned int i = 0; i < N; ++i) {
            double lambda = svd.singularValues()[i];
            if (lambda < -CASEPS) {
                throw std::runtime_error("CopulaAlphaStable:: rho is not positive semidefinite");
            }
            lambda = std::max(lambda, 0.);
            if (sumvar < maxvar && M < maxM) {
                sumvar += lambda;
                ++M;
            }
        }
        
        if (sumvar < maxvar * (1 - 1E-8) || M > maxM) {
            throw std::runtime_error("CopulaAlphaStable: variance conditions impossible to satisfy");
        }

        _S = svd.matrixU().block(0, 0, N, M);
        for (unsigned int i = 0; i < M; ++i) {
            _S.col(i) *= sqrt(std::max(svd.singularValues()[i] * N / sumvar, 0.0));
        }
        MoorePenrose::inverse(_S, CASEPS, _invS);
        assert(_invS.rows() == _S.cols());
        assert(_invS.cols() == _S.rows());
    }

	CopulaAlphaStable::CopulaAlphaStable(CopulaAlphaStable&& other)
		: CopulaMultifactor(std::move(other)),
		_S(std::move(other._S)),
		_invS(std::move(other._invS)),
		_alpha(other._alpha) {		
	}
        
    void CopulaAlphaStable::draw_corr_factors(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        rng.next_alpha_stable(_alpha, _S, x);
    }
    
    double CopulaAlphaStable::scale() const {
        if (_alpha == 2) {
            static const double sqrt2 = sqrt(2.0);
            return sqrt2;
        } else {
            return 1.0;
        }
    }
    
    double CopulaAlphaStable::marginal_factor_cdf(double x) const {
        if (_alpha == 2) {
            return NormalDistribution::normcdf(x);
        } else if (_alpha == 1) {
            return CauchyDistribution::cdf(x);
        } else {
            throw std::runtime_error("CopulaAlphaStable: CDF not implemented for alpha != 1 or 2");
        }
    }
    
    double CopulaAlphaStable::marginal_factor_icdf(double p) const {
        if (_alpha == 2) {
            return NormalDistribution::normsinv(p);
        } else if (_alpha == 1) {
            return CauchyDistribution::icdf(p);
        } else {
            throw std::runtime_error("CopulaAlphaStable: inverse CDF not implemented for alpha != 1 or 2");
        }
    }
    
    void CopulaAlphaStable::adjust_cdfs(Eigen::Ref<Eigen::MatrixXd> sample) const {
        if (sample.cols() != _S.rows()) {
            throw std::domain_error("CopulaAlphaStable: bad sample dimension");
        }
        std::transform(sample.data(), sample.data() + sample.size(), sample.data(), [this](double p){ return marginal_factor_icdf(p); });
        Eigen::MatrixXd iid_sample(sample.rows(), _S.cols());
        // sample^T = S * iid_sample^T
        // iid_sample^T = invS * sample^T
        // iid_sample = sample * invS^T
        iid_sample.noalias() = sample * _invS.transpose();
        // std::transform(iid_sample.data(), iid_sample.data() + iid_sample.size(), iid_sample.data(), [this](double x){ return marginal_factor_cdf(x); }); // not needed because CDF is monotonically increasing
        Statistics::percentiles_inplace(iid_sample);
        std::transform(iid_sample.data(), iid_sample.data() + iid_sample.size(), iid_sample.data(), [this](double p){ return marginal_factor_icdf(p); });
        sample.noalias() = iid_sample * _S.transpose();
        std::transform(sample.data(), sample.data() + sample.size(), sample.data(), [this](double x){ return marginal_factor_cdf(x); });
    }
}
