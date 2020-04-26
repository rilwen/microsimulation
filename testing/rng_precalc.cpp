// (C) Averisera Ltd 2014-2020
#include "rng_precalc.hpp"
#include <algorithm>
#include <limits>
#include <stdexcept>
#include "core/normal_distribution.hpp"

namespace averisera {
    namespace testing {
        RNGPrecalc::RNGPrecalc(const std::vector<double>& sample)
            : _sample(sample) {
            if (std::any_of(_sample.begin(), _sample.end(), [](double u) { return u < 0. || u > 1.; })) {
                throw std::domain_error("RNGPrecalc: U(0,1) number of out range");
            }
            reset();
        }

        double RNGPrecalc::next_uniform() {
            if (!has_next()) {
                throw std::runtime_error("RNGPrecalc: ran out of juice");
            }
            return *(_iter++);
        }

        double RNGPrecalc::next_gaussian() {
            if (!has_next()) {
                throw std::runtime_error("RNGPrecalc: ran out of juice");
            }
            return NormalDistribution::normsinv(*(_iter++));
        }

        uint64_t RNGPrecalc::next_uniform(uint64_t n) {
            return std::min(static_cast<uint64_t >(static_cast<double>(n) * next_uniform()), n);
        }

        double RNGPrecalc::next_alpha_stable(double) {
            throw std::logic_error("RNGPrecalc: not implemented");
        }

        void RNGPrecalc::next_gaussians(const Eigen::MatrixXd&, Eigen::Ref<Eigen::VectorXd>) {
            throw std::logic_error("RNGPrecalc: not implemented");
        }

        void RNGPrecalc::next_gaussians_noncont(const Eigen::MatrixXd&, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>>) {
            throw std::logic_error("RNGPrecalc: not implemented");
        }

        void RNGPrecalc::next_alpha_stable(double, const Eigen::MatrixXd&, Eigen::Ref<Eigen::VectorXd>) {
            throw std::logic_error("RNGPrecalc: not implemented");
        }

		RNGPrecalc::int_type RNGPrecalc::rand_int() {
            return static_cast<int_type>(next_uniform() * static_cast<double>(int_max()));
        }

        void RNGPrecalc::discard(const unsigned long long z) {
            for (unsigned long long i = 0; i < z; ++i) {
                if (!has_next()) {
                    throw std::runtime_error("RNGPrecalc: ran out of juice");
                }
                ++_iter;
            }
        }        
    }
}
