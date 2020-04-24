#ifndef __AVERISERA_DISTRIBUTION_CONDITIONAL_H
#define __AVERISERA_DISTRIBUTION_CONDITIONAL_H

#include "distribution.hpp"
#include <memory>

namespace averisera {
    /** Conditional distribution: P(X|X \in [a,b)) */
    class DistributionConditional: public Distribution {
    public:
        /** Given distribution of X, construct distribution of Y := X | X \in [a, b)
          @param orig Distribution of X
          @throw std::domain_error If orig is null or b <= a 
          @throw std::runtime_error If P(X \in [a, b)) = 0
        */
        DistributionConditional(std::shared_ptr<const Distribution> orig, double a, double b);

        /** Given distribution of X, construct distribution of Y := X | X \in [a, b)
          @param orig Distribution of X
          @param mean Conditional mean
          @param variance Conditional variance
          @throw std::domain_error If orig is null, variance < 0, mean not in [a, b) or b <= a 
          @throw std::runtime_error If P(X \in [a, b)) = 0
        */
        DistributionConditional(std::shared_ptr<const Distribution> orig, double a, double b, double mean, double variance);

        double pdf(double x) const override;

        double cdf(double x) const override;

        double cdf2(double x) const override;

        double range_prob(double x1, double x2) const override;

        double range_prob2(double x1, double x2) const override;

        double icdf(double p) const override;

        double mean() const override {
            return _mean;
        }

        double variance(double mean) const override {
            return _variance;
        }

        std::unique_ptr<Distribution> clone() const;

        /** Unconditional probability P(X \in [a, b)) */
        double prob() const {
            return _p_ab;
        }

		double infimum() const override {
			return _a;
		}

		double supremum() const override {
			return _b;
		}
    private:
        void validate_moments() const;
        
        std::shared_ptr<const Distribution> _orig;
        double _a;
        double _b;
        double _cdf2_a; /**< P(X < a) */
        double _cdf2_b; /**< P(X < b) */
        double _p_ab; /**< P(X \in [a, b)) */
        double _mean;
        double _variance;
    };
}

#endif // __AVERISERA_DISTRIBUTION_CONDITIONAL_H
