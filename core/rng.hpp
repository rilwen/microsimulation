/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_RNG_H
#define __AVERISERA_RNG_H

#include <cstdint>
#include <vector>
#include "eigen.hpp"

namespace averisera {
    /** Random number generator. To be compatible with STL static variables, we fix the word size at 64 bit. */
    class RNG {
    public:
		typedef uint64_t int_type;

        virtual ~RNG();
        
        /** Draw a U(0, 1) number */
        virtual double next_uniform() = 0;
        
        /** Draw a N(0, 1) number */
        virtual double next_gaussian() = 0;

        /** Draw a number uniformly distributed between 0 and n (inclusive) */
        virtual int_type next_uniform(int_type n) = 0;

        /** Draw a number from symmetric alpha-stable distribution with scale parameter 1 (for alpha != 2) or 1/sqrt(2) (for alpha == 2)
         * @param alpha In (0, 2]
         */
        virtual double next_alpha_stable(double alpha) = 0;

        /** Draw correlated Gaussian variables
        @param[in] S N x M matrix converting M i.i.d. Gaussians into N correlated Gaussians.
        @param[out] y Vector of size N.
        @throw std::domain_error If S.rows() != y.size()
        */
        void next_gaussians(const Eigen::MatrixXd& S, std::vector<double>& y) {
            next_gaussians(S, EigenUtils::from_vec(y));
        }

	/** Draw correlated Gaussian variables
        @param[in] S N x M matrix converting M i.i.d. Gaussians into N correlated Gaussians.
        @param[out] y Vector-like object with continuous storage of size N.
        @throw std::domain_error If S.rows() != y.size()
        */
        virtual void next_gaussians(const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd> y) = 0;

	/** Draw correlated Gaussian variables
        @param[in] S N x M matrix converting M i.i.d. Gaussians into N correlated Gaussians.
        @param[out] y Vector-like object of size N.
        @throw std::domain_error If S.rows() != y.size()
        */
        virtual void next_gaussians_noncont(const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> y) = 0;
        
        /** Draw correlated alpha-stable variables 
        @param alpha In (0, 2]
        @param[in] S N x M matrix converting M i.i.d. alpha-stable variables 
        with scale 1 (alpha != 2) or 1/sqrt(2) (alpha == 2) into N correlated alpha-stable variables.
        @param[out] y Vector of size N.
        @throw std::domain_error If S.rows() != y.size()
        */
        void next_alpha_stable(double alpha, const Eigen::MatrixXd& S, std::vector<double>& y) {
            return next_alpha_stable(alpha, S, EigenUtils::from_vec(y));
        }

        /** Draw correlated alpha-stable variables 
        @param alpha In (0, 2]
        @param[in] S N x M matrix converting M i.i.d. alpha-stable variables 
        with scale 1 (alpha != 2) or 1/sqrt(2) (alpha == 2) into N correlated alpha-stable variables.
        @param[out] y Vector of size N.
        @throw std::domain_error If S.rows() != y.size()
        */
        virtual void next_alpha_stable(double alpha, const Eigen::MatrixXd& S, Eigen::Ref<Eigen::VectorXd> y) = 0;

        /** Uniformly random integer from range [int_min(), int_max()] */
        virtual int_type rand_int() = 0;

        /** Min. value returned by rand_int() */
		static constexpr int_type int_min() {
			return 0u;
		}

        /** Max. value returned by rand_int() */
		static constexpr int_type int_max() {
			return std::numeric_limits<int_type>::max();
		}

        /** Advance by z random numbers in the sequence, discarding them. */
        virtual void discard(unsigned long long z) = 0;

		/** Return true with probability p and false with 1-p 
		@param p In [0, 1] range (not checked)
		*/
		virtual bool flip(double p = 0.5);

        /** Wraps RNG implementation to make it work like STL random number generators */
        class StlWrapper {
        public:
            typedef int_type result_type;
            StlWrapper(RNG& rng)
                : _rng(rng) {}

            StlWrapper(const StlWrapper&) = default;

            StlWrapper& operator=(const StlWrapper&) = delete;

            static constexpr result_type min() {
				return int_min();
            }

            static constexpr result_type max() {
                return int_max();
            }

            void discard(unsigned long long z) {
                _rng.discard(z);
            }

            result_type operator()() {
                return _rng.rand_int();
            }            
        private:
            RNG& _rng;
        };
	private:
    };
}


#endif // __AVERISERA_RNG_H
