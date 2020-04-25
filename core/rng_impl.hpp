/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_RNG_IMPL_H
#define __AVERISERA_RNG_IMPL_H

#include "rng.hpp"
#include <gtest/gtest_prod.h>
#include <random>

namespace averisera {
    /** Default implementation of RNG */
    class RNGImpl: public RNG {
    public:
        RNGImpl(long seed = 42);
        
        double next_uniform() override;
        
        double next_gaussian() override;

        uint64_t next_uniform(uint64_t n) override;
        
        double next_alpha_stable(double alpha) override;

        void next_gaussians(const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd> y) override;
        
        void next_gaussians_noncont(const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> y) override;
        
        void next_alpha_stable(double alpha, const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd>) override;

        int_type rand_int() override {
            return _rng();
        }

        void discard(unsigned long long z) override {
            _rng.discard(z);
        }

		
    private:
        std::mt19937_64 _rng;
        std::uniform_real_distribution<double> _u01;
        std::normal_distribution<double> _n01;

		std::string to_string() const;
    };
}

#endif // __AVERISERA_RNG_IMPL_H
