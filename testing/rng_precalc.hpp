#ifndef __AVERISERA_TESTING_RNG_PRECALC_H
#define __AVERISERA_TESTING_RNG_PRECALC_H
#include "core/rng.hpp"
#include <vector>

namespace averisera {
    namespace microsim {
        class Contexts;
    }
    namespace testing {
        /*! \brief RNG implementation using precalculated "random" uniform numbers. Throws when ran out of numbers. */
        class RNGPrecalc: public RNG {
        public:
            RNGPrecalc(const std::vector<double>& sample);

            double next_uniform() override;

            double next_gaussian() override;

            double next_alpha_stable(double alpha) override;

            uint64_t next_uniform(uint64_t n) override;

            // throws
            void next_gaussians(const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd> y) override;

            // throws
            void next_gaussians_noncont(const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> y) override;

            // throws
            void next_alpha_stable(double alpha, const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd> y) override;

            /*! Check if we have some numbers left */
            bool has_next() const {
                return _iter != _sample.end();
            }

            /*! Reset the stream of random numbers */
            void reset() {
                _iter = _sample.begin();
            }

            int_type rand_int() override;

            void discard(unsigned long long z) override;
        private:
            std::vector<double> _sample;
            std::vector<double>::const_iterator _iter;
        };

    }
}

#endif 
