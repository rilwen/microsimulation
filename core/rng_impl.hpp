/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_RNG_IMPL_H
#define __AVERISERA_RNG_IMPL_H

#include "rng.hpp"
#include "serialization.hpp"
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/nvp.hpp>
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
		FRIEND_TEST(RNGImplTest, SerializeDirectly);
		FRIEND_TEST(RNGImplTest, SerializeViaUniquePtr);
        std::mt19937_64 _rng;
        std::uniform_real_distribution<double> _u01;
        std::normal_distribution<double> _n01;

		friend class boost::serialization::access;
		std::string to_string() const;
		/** For boost::serialization */
		template <class Archive> void serialize(Archive& ar, const unsigned int version) {
			// save/load base class information
			ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(RNG);
			ar & _rng;
		}
    };

	///** For boost::serialization */
	//BOOST_CLASS_EXPORT(RNGImpl)
}

#endif // __AVERISERA_RNG_IMPL_H
