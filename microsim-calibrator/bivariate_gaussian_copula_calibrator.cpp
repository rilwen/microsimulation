// (C) Averisera Ltd 2014-2020
#include "bivariate_gaussian_copula_calibrator.hpp"
#include <cassert>
#include <cmath>
#include <stdexcept>
#include "core/adapt.hpp"
#include "core/brent.hpp"
#include "core/distribution.hpp"
#include "core/normal_distribution.hpp"

namespace averisera {
    namespace microsim {
        BivariateGaussianCopulaCalibrator::BivariateGaussianCopulaCalibrator(double tol, unsigned int maxpts_to_rulcls)
            : _tol(tol), _maxpts_to_rulcls(maxpts_to_rulcls) {
            if (tol <= 0) {
                throw std::domain_error("BivariateGaussianCopulaCalibrator: tol not positive");
            }
        }

        /** Integrand class for AdaptIntegration */
        class BivariateGaussianCopulaCalibratorIntegrand {
        public:
            BivariateGaussianCopulaCalibratorIntegrand(double mean_x, double mean_y, const Distribution& x, const Distribution& y)
                : _mean_x(mean_x), _mean_y(mean_y), _x(x), _y(y) {
                set_r(0);
            }

            void set_r(double r) {
                assert(fabs(r) <= 1 && "r must be within [-1, 1]");
                _r = r;
                _sqrt_1pr = sqrt(1 + r);
                _sqrt_1mr = sqrt(1 - r);
            }
            
            double operator()(double zp, double zm) const {
                return (_x.icdf(fm(zp, zm)) - _mean_x) * (_y.icdf(fp(zp, zm)) - _mean_y);
            }
        private:
            double fp(double zp, double zm) const {
                const double y = (NormalDistribution::normsinv(zp) * _sqrt_1pr + NormalDistribution::normsinv(zm) * _sqrt_1mr) / sqrt2();
                return NormalDistribution::normcdf(y);
            }

            double fm(double zp, double zm) const {
                const double y = (NormalDistribution::normsinv(zp) * _sqrt_1pr - NormalDistribution::normsinv(zm) * _sqrt_1mr) / sqrt2();
                return NormalDistribution::normcdf(y);
            }
        private:
#ifndef _WIN32
            static constexpr double sqrt2() {
                return sqrt(2.0);
            }
#else
            // sqrt() is not constexpr in Visual Studio 2015
            static double sqrt2() {
                return 1.4142135623730950488016887242097;
            }
#endif // _WIN32
            
            const double _mean_x;
            const double _mean_y;
            const Distribution& _x;
            const Distribution& _y;
            double _r;
            double _sqrt_1pr;
            double _sqrt_1mr;
        };

        /** Calculates correlation for root solver */
        class BivariateGaussianCopulaCalibratorFunction {
        public:
            BivariateGaussianCopulaCalibratorFunction(double mean_x, double mean_y, const Distribution& x, const Distribution& y, double target_rho,
                                                      double var_x, double var_y, double integration_eps, unsigned int maxpts_to_rulcls)
                : _integrand(mean_x, mean_y, x, y), _ai(2, integration_eps, maxpts_to_rulcls),  _target_rho(target_rho), _rho_denom(sqrt(var_x * var_y)) {
            }

            double operator()(double r) {
                _integrand.set_r(r);
                double relerr;
                int ifail;
                const double cov_xy = _ai.integrate(integrand_func, &_integrand, relerr, ifail);
                const double rho = cov_xy / _rho_denom;
                return rho - _target_rho;
            }
        private:
            static double integrand_func(const int* /*dim*/, const double* z, const int* /*npara*/, const double* params) {
                const void * p = AdaptIntegration::get_ptr(params[0]);
                assert(p);
                const BivariateGaussianCopulaCalibratorIntegrand* integrand_ptr = static_cast<const BivariateGaussianCopulaCalibratorIntegrand*>(p);
                return (*integrand_ptr)(z[0], z[1]);
            }
                        
            BivariateGaussianCopulaCalibratorIntegrand _integrand;
            AdaptIntegration _ai;
            double _target_rho;
            double _rho_denom;
        };
        
        double BivariateGaussianCopulaCalibrator::calibrate(const double rho, std::shared_ptr<const Distribution> x, std::shared_ptr<const Distribution> y) const {
            if (rho == 0.0) {
                return 0.0;
            }
            const double mean_x = x->mean();
            const double mean_y = y->mean();
            const double var_x = x->variance(mean_x);
            const double var_y = y->variance(mean_y);
            if (var_x == 0 || var_y == 0) {
                return 0.0;
            }
            BivariateGaussianCopulaCalibratorFunction fun(mean_x, mean_y, *x, *y, rho, var_x, var_y, _tol, _maxpts_to_rulcls);
            double r;
            if (rho > 0) {
                r = RootFinding::find_root(fun, 0, 1, _tol, _tol);
            } else {
                r = RootFinding::find_root(fun, -1, 0, _tol, _tol);
            }
            return r;
        }

        double BivariateGaussianCopulaCalibrator::calculate_rho(const double r, std::shared_ptr<const Distribution> x, std::shared_ptr<const Distribution> y) const {
            const double mean_x = x->mean();
            const double mean_y = y->mean();
            const double var_x = x->variance(mean_x);
            const double var_y = y->variance(mean_y);
            if (var_x == 0 || var_y == 0) {
                return 0.0;
            }
            BivariateGaussianCopulaCalibratorFunction fun(mean_x, mean_y, *x, *y, 0.0, var_x, var_y, _tol, _maxpts_to_rulcls);
            return fun(r);
        }
    }
}
