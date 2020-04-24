#ifndef __AVERISERA_COPULA_MULTIFACTOR_H
#define __AVERISERA_COPULA_MULTIFACTOR_H

#include "copula.hpp"

namespace averisera {
    /** Multi-factor copula: correlated variables are mapped onto correlated factors with known distribution. */
    class CopulaMultifactor: public Copula {
    public:
        /** @param dim Copula dimension
         */
        CopulaMultifactor(size_t dim);

		CopulaMultifactor(CopulaMultifactor&& other);
                
        void draw_cdfs(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;
        
        size_t dim() const override {
            return _dim;
        }
    private:
        virtual void draw_corr_factors(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const = 0;
        virtual double marginal_factor_cdf(double x) const = 0;
        virtual double marginal_factor_icdf(double p) const = 0;
        
        size_t _dim;
    };
}

#endif // __AVERISERA_COPULA_MULT|FACTOR_H
