// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MULTIVARIATE_DISTRIBUTION_H
#define __AVERISERA_MULTIVARIATE_DISTRIBUTION_H

#include <stdexcept>
#include <vector>
#include "eigen.hpp"

namespace averisera {
    class RNG;
    
    /** Real-valued multivariate distribution */
    class MultivariateDistribution {
    public:
        virtual ~MultivariateDistribution();
        
        /** Dimension of the distribution */
        virtual size_t dim() const = 0;
        
        /** Draw a sample.
         * @param rng Random number generator
         * @param x Sample vector with x.size() == dim()
         * @throw std::domain_error If x.size() != dim()
         */
        void draw(RNG& rng, std::vector<double>& x) const {
            draw(rng, EigenUtils::from_vec(x));
        }

        /** Draw a sample.
         * @param rng Random number generator
         * @param x Sample vector expression with continuous layout and x.size() == dim()
         * @throw std::domain_error If x.size() != dim()
         */
        virtual void draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const = 0;

        /** Draw a sample.
         * @param rng Random number generator
         * @param x Sample vector expression with not necessarily continuous layout and x.size() == dim()
         * @throw std::domain_error If x.size() != dim()
         */
        virtual void draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const = 0;
        
        /** Calculate marginal CDF values. p and x can be the same vector.
         * @param[in] x Random variable values
         * @param[out] p Marginal CDF values
         * @throw std::domain_error If x.size() != dim() or p.size() != dim()
         */
        void marginal_cdf(const std::vector<double>& x, std::vector<double>& p) const {
            marginal_cdf(EigenUtils::from_vec(x), EigenUtils::from_vec(p));
        }

        /** Calculate marginal CDF values. p and x can be the same vector.
         * @param[in] x Random variable values
         * @param[out] p Marginal CDF values
         * @throw std::domain_error If x.size() != dim() or p.size() != dim()
         */
        virtual void marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const = 0;
        
        /** Calculate marginal inverse CDF values. p and x can be the same vector.
         * @param[in] p Marginal CDF values
         * @param[out] x Random variable values
         * @throw std::domain_error If x.size() != dim() or p.size() != dim()
         */
        void marginal_icdf(const std::vector<double>& p, std::vector<double>& x) const {
            marginal_icdf(EigenUtils::from_vec(p), EigenUtils::from_vec(x));
        }

        /** Calculate marginal inverse CDF values. p and x can be the same vector.
         * @param[in] p Marginal CDF values
         * @param[out] x Random variable values
         * @throw std::domain_error If x.size() != dim() or p.size() != dim()
         */
        virtual void marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const = 0;

        /** Adjust the distribution in the sample to match the distribution.
          @param[in,out] sample Sample matrix with points arranged row by row.
          @throw std::domain_error If dim() != sample.cols()
        */
        virtual void adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const = 0;
    };
}

#endif // __AVERISERA_MULTIVARIATE_DISTRIBUTION_H
