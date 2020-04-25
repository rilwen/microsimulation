#ifndef __AVERISERA_MS_BIVARIATE_GAUSSIAN_COPULA_CALIBRATOR_H
#define __AVERISERA_MS_BIVARIATE_GAUSSIAN_COPULA_CALIBRATOR_H

#include <memory>

namespace averisera {
    class Distribution;
    
    namespace microsim {
        /** Calibrates a bivariate Gaussian copula to a correlation between 2 known distributions */
        class BivariateGaussianCopulaCalibrator {
        public:
            /** @param tol Relative tolerance
              @param maxpts_to_rulcls @see AdaptIntegration
              @throw std::domain_error If tol <= 0
            */
            BivariateGaussianCopulaCalibrator(double tol, unsigned int maxpts_to_rulcls = 1000);

            double calibrate(double rho, std::shared_ptr<const Distribution> x, std::shared_ptr<const Distribution> y) const;

            /** Calculate rho corresponding to given copula parameter r */
            double calculate_rho(double r, std::shared_ptr<const Distribution> x, std::shared_ptr<const Distribution> y) const;
        private:
            double _tol;
            unsigned int _maxpts_to_rulcls;
        };
    }
}

#endif // __AVERISERA_MS_BIVARIATE_GAUSSIAN_COPULA_CALIBRATOR_H
