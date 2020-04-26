// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_DISTRIBUTION_EXPONENTIAL_H
#define __AVERISERA_DISTRIBUTION_EXPONENTIAL_H

#include "distribution.hpp"
#include <cmath>

namespace averisera {
    /** Distribution with density f(x) ~ exp(-lambda(sgn(x)) * abs(x)) */
    class DistributionExponential: public Distribution {
    public:
        /** @param lambda_neg Lambda for x < 0
         * @param lambda_pos Lambda for x >= 0
         * @throw std::out_of_range If either lambda is negative or zero
         */
        DistributionExponential(double lambda_neg, double lambda_pos);
        
        double pdf(double x) const override {
            if (x < 0) {
                return exp(_lambda_neg * x) / _norm;
            } else {
                return exp(- _lambda_pos * x) / _norm;
            }
        }
            
        double cdf(double x) const override;
        
        double icdf(double p) const override;

        double mean() const override {
            return _mean;
        }

        double variance(double /*mean*/) const override {
            return _variance;
        }

		double infimum() const override;

		double supremum() const override;
    private:
        double _lambda_neg;
        double _lambda_pos;
        double _norm; /**< Normalisation factor */
        double _cdf0; /** CDF(0) */
        double _mean;
        double _variance;
    };
}

#endif // __AVERISERA_DISTRIBUTION_EXPONENTIAL_H
