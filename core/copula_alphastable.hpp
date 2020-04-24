#ifndef __AVERISERA_COPULA_ALPHASTABLE_H
#define __AVERISERA_COPULA_ALPHASTABLE_H

#include "copula_multifactor.hpp"
#include <Eigen/Core>
#include <vector>

namespace averisera {
    /** Multi-factor copula built from i.i.d. alpha-stable variables with scale 1 (for alpha != 2)
     * or 1/sqrt(2) (for alpha = 2). */
    class CopulaAlphaStable: public CopulaMultifactor {
    public:
        /** @param alpha Alpha parameter
         * @param S Transformation matrix: Y = S * Z, where Z are i.i.d. alpha-stable variables. S rows are rescaled so that each Y_i has scale = scale().
         * @throw std::out_of_range If alpha <= 0 or alpha > 2
         */
        CopulaAlphaStable(double alpha, const Eigen::MatrixXd& S);
        
        /** Version with 1 common factors and N intrinsic factors, which creates N correlated factors with scale = scale()
         *  @param alpha Alpha parameter
         * @param factor_loadings Coefficients for intrinsic factors
         * @throw std::out_of_range If alpha <= 0 or alpha > 2, or if any factor loading outside [-1, 1]
         */
        CopulaAlphaStable(double alpha, const std::vector<double>& factor_loadings);
        
        /** @brief Construct Gaussian copula from given correlation matrix of correlated Gaussians.
         * 
         * Capture at least min_variance_fract of the total variance, using at most max_nbr_factors factors.
         * 
         * @param[in] rho Correlation matrix
         * @param[in] min_variance_fract Minimum fraction of total variance to be captured
         * @param[in] max_nbr_factors Maximum number of factors to be used. If zero, assume no maximum.
         * @throw std::runtime_error If conditions impossible to satisfy or rho is not positive semidefinite.
         * @throw std::domain_error If rho is not a square matrix, or min_variance_fract is above 1.0.
         */
        CopulaAlphaStable(const Eigen::MatrixXd& rho, double min_variance_fract = 1.0, unsigned int max_nbr_factors = 0);

		CopulaAlphaStable(CopulaAlphaStable&& other);
        
        double alpha() const {
            return _alpha;
        }
        
        /** Scale of correlated factors 
         * 1 for alpha != 2
         * 1/sqrt(2) for alpha == 2
         */
        double scale() const;
        
        /** Number of independent factors */
        size_t nbr_independent_factors() const {
            return static_cast<size_t>(_S.cols());
        }
        
        const Eigen::MatrixXd& S() const {
            return _S;
        }
        
        void adjust_cdfs(Eigen::Ref<Eigen::MatrixXd> sample) const override;
    private:
        void draw_corr_factors(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;
        double marginal_factor_cdf(double x) const override;
        double marginal_factor_icdf(double p) const override;
        
        Eigen::MatrixXd _S;
        Eigen::MatrixXd _invS; /**< Pseudo-inverse of S */
		double _alpha;
    };
}

#endif // __AVERISERA_COPULA_ALPHASTABLE_H
