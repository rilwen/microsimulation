#ifndef __AVERISERA_COPULA_GAUSSIAN_H
#define __AVERISERA_COPULA_GAUSSIAN_H

#include "copula_alphastable.hpp"
#include "multivariate_distribution_gaussian_simple.hpp"
#include "multivariate_distribution_transformed.hpp"
#include "normal_distribution.hpp"
#include <cassert>
#include <memory>
#include <boost/format.hpp>

namespace averisera {
    class Distribution;
	class CSVFileReader;
    
    /**
      Gaussian copula
    */
    class CopulaGaussian: public CopulaAlphaStable {
    public:
        /** @see CopulaAlphaStable */
        CopulaGaussian(const Eigen::MatrixXd& rho, double min_variance_fract = 1.0, unsigned int max_nbr_factors = 0);

		CopulaGaussian(CopulaGaussian&& other);
		
		static CopulaGaussian from_sample(const Eigen::MatrixXd& sample, double min_variance_fract = 1.0, unsigned int max_nbr_factors = 0);

		static CopulaGaussian from_sample(CSVFileReader& reader, const std::vector<size_t>& indices, double min_variance_fract = 1.0, unsigned int max_nbr_factors = 0);

		static CopulaGaussian from_sample(CSVFileReader& reader, const std::vector<std::string>& indices, double min_variance_fract = 1.0, unsigned int max_nbr_factors = 0);

        const Eigen::MatrixXd& rho() const {
            return _rho;
        }		

        /** Conditioning.
          
          1. Apply a particular set of marginal distributions to create a multivariate distribution.
          2. Condition this distribution on some correlated variables being set.
          @param marginals_begin Beginning (inclusive) of the range of shared pointers to marginal distributions.
          @param marginals_end End (exclusive) of the range of shared pointers to marginal distributions.
          @param x Vector with specified variables, of size dim(). Unspecified ones should be set to NaN.
          @throw std::domain_error If x.size() != dim() or marginals.size() != dim()
        */
        template <class I> std::unique_ptr<MultivariateDistribution> conditional(I marginals_begin, const I marginals_end, Eigen::Ref<const Eigen::VectorXd> x) const;

        /** Conditioning (just the internal Gaussian distribution).
          @param a Vector with specified Gaussian factor values, of size dim(). Unspecified ones should be set to NaN.
          @param cond_dim Precalculated conditional dimension (number of NaNs in a).
          @throw std::domain_error If a.size() != dim() or cond_dim > dim()
         */
        std::unique_ptr<MultivariateDistributionGaussianSimple> conditional(Eigen::Ref<const Eigen::VectorXd> a, unsigned int cond_dim) const {
            return std::unique_ptr<MultivariateDistributionGaussianSimple>(make_conditional(a, cond_dim));
        }
    private:
        MultivariateDistributionGaussianSimple* make_conditional(Eigen::Ref<const Eigen::VectorXd> a, unsigned int cond_dim) const;
        
        Eigen::MatrixXd _rho;
    };

    template <class I> std::unique_ptr<MultivariateDistribution> CopulaGaussian::conditional(I marginals_begin, const I marginals_end, Eigen::Ref<const Eigen::VectorXd> x) const {
        if (std::distance(marginals_begin, marginals_end) != static_cast<ptrdiff_t>(dim()) || static_cast<unsigned int>(x.size()) != dim()) {
            throw std::domain_error("CopulaGaussian: size mismatch");
        }
        Eigen::VectorXd a(dim());
        std::vector<std::function<double(double)>> transforms;
        std::vector<std::function<double(double)>> inverse_transforms;
        transforms.reserve(dim());
        inverse_transforms.reserve(dim());
        for (unsigned int i = 0; i < dim(); ++i, ++marginals_begin) {
            assert(marginals_begin != marginals_end);
            // important to take copies of shared pointers here
            const std::shared_ptr<const Distribution> marginal = *marginals_begin;
            if (!marginal) {
                throw std::domain_error(boost::str(boost::format("CopulaGaussian: null marginal at index %d") % i));
            }
            const std::function<double(double)> inverse_transform([marginal](double y){ return NormalDistribution::normsinv(marginal->cdf(y)); });
            if (std::isnan(x[i])) {
                a[i] = x[i];                
                transforms.push_back(std::function<double(double)>([marginal](double x){ return marginal->icdf(NormalDistribution::normcdf(x)); }));
                inverse_transforms.push_back(inverse_transform);
            } else {                
                a[i] = inverse_transform(x[i]);
            }
        }
        const unsigned int cond_dim = static_cast<unsigned int>(transforms.size());
        return std::unique_ptr<MultivariateDistribution>(new MultivariateDistributionTransformed(make_conditional(a, cond_dim), std::move(transforms), std::move(inverse_transforms)));
    }

    
}

#endif // __AVERISERA_COPULA_GAUSSIAN_H
