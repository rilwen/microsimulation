/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_COPULA_H
#define __AVERISERA_COPULA_H

#include <vector>
#include "eigen.hpp"

namespace averisera {
    class RNG;
    
    /** @brief Statistical copula */
    class Copula {
    public:
        virtual ~Copula();
        
        // /** Calculate joint CDF from marginal CDFs 
        // * 
        // * @param[in] marginal_cdfs Vector of marginal CDF values.
        // * @throw std::domain_error If the size of the vector != dim()
        // * @return A double number in [0, 1] range
        // */
        //virtual double joint_cdf(const std::vector<double>& marginal_cdfs) const = 0;
        
        /** Number of correlated variables */
        virtual size_t dim() const = 0;

        /** Draw marginal CDFs
         * @param[in] rng Random number generator
         * @param[out] x Vector with size() == dim()
         * @throw std::domain_error If x.size() != dim()
         */
        void draw_cdfs(RNG& rng, std::vector<double>& x) const {
            draw_cdfs(rng, EigenUtils::from_vec(x));
        }

        virtual void draw_cdfs(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const = 0;
        
        /** Adjust the CDFs in the sample to match the copula distribution
         * @param[in,out] sample Sample matrix with points arranged row by row.
         * @throw std::domain_error If dim() != sample.cols()
        */
        virtual void adjust_cdfs(Eigen::Ref<Eigen::MatrixXd> sample) const = 0;
    };
}

#endif // __AVERISERA_MS_COPULA_H
