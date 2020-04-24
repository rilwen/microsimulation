/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_COPULA_INDEPENDENT_H
#define __AVERISERA_COPULA_INDEPENDENT_H

#include "copula.hpp"

namespace averisera {
    /** Independent copula */
    class CopulaIndependent: public Copula {
    public:
        /** @param[in] dim Dimension of the copula */
        CopulaIndependent(size_t dim);
        
        //double joint_cdf(const std::vector<double>& marginal_cdfs) const;
        
        size_t dim() const override {
            return _dim;
        }
        
        void draw_cdfs(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;
        
        void adjust_cdfs(Eigen::Ref<Eigen::MatrixXd> sample) const override;
    private:
        size_t _dim;
    };
}

#endif // __AVERISERA_COPULA_INDEPENDENT_H
