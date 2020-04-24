/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_DISTRIBUTION_TRANSFORMED_H
#define __AVERISERA_DISTRIBUTION_TRANSFORMED_H

#include "distribution.hpp"
#include <functional>
#include <memory>

namespace averisera {
    /** Distribution constructed by taking another distribution and applying a strictly increasing function to it */
    class DistributionTransformed: public Distribution {
        public:
            /**
             * @param orig Pointer to original distribution (constructed object takes ownership)
             * @param transform Transformation function this = F(orig)
             * @param transform_derivative Derivative of transform
             * @param inverse_transform Inverse of transform
             * @throw std::domain_error If orig is null
             */
            DistributionTransformed(const Distribution* orig, std::function<double(double)> transform, std::function<double(double)> transform_derivative, std::function<double(double)> inverse_transform);
            
            double pdf(double x) const override;
            
            double cdf(double x) const override;

			double cdf2(double x) const override;
            
            double range_prob(double x1, double x2) const override;
            
            double icdf(double p) const override;

			double infimum() const override;

			double supremum() const override;
        private:
            std::unique_ptr<const Distribution> _orig;
            std::function<double(double)> _transform;
            std::function<double(double)> _transform_derivative;
            std::function<double(double)> _inverse_transform;            
    };
}

#endif // __AVERISERA_DISTRIBUTION_TRANSFORMED_H

