#pragma once
/*
(C) Averisera Ltd
*/
#include <memory>
#include <random>
#include <boost/serialization/split_free.hpp>

/** Serialisation of external classes using Boost::Serialization */

namespace boost {
	namespace serialization {

		// based on https://stackoverflow.com/a/45873005/59557
#define MT_TPARAMS typename UIntType, size_t w, size_t n, size_t m, size_t r, UIntType a, size_t u, UIntType d, size_t s, UIntType b, size_t t, UIntType c, size_t l, UIntType f
#define MT_TARGLIST UIntType, w, n, m, r, a, u, d, s, b, t, c, l, f

		template<typename Ar, MT_TPARAMS>
		void load(Ar& ar, std::mersenne_twister_engine<MT_TARGLIST>& mt, unsigned int) {
			std::string text;
			ar & text;
			std::istringstream iss(text);

			if (!(iss >> mt))
				throw std::invalid_argument("mersenne_twister_engine state");
		}

		template<typename Ar, MT_TPARAMS>
		void save(Ar& ar, std::mersenne_twister_engine<MT_TARGLIST> const& mt, unsigned int) {
			std::ostringstream oss;
			if (!(oss << mt))
				throw std::invalid_argument("mersenne_twister_engine state");
			std::string text = oss.str();
			ar & text;
		}

		template<typename Ar, MT_TPARAMS>
		inline void serialize(Ar& ar, std::mersenne_twister_engine<MT_TARGLIST>& mt, const unsigned int version) {
			boost::serialization::split_free(ar, mt, version);
		}		

#undef MT_TPARAMS
#undef MT_TARGLIST

		/* unique_ptr */

		template <class Archive, class T> inline void load(Archive& ar, std::unique_ptr<T>& ptr, unsigned int) {
			T* ptr_target;
			ar >> ptr_target;
			ptr.reset(ptr_target);
		}

		template <class Archive, class T> inline void save(Archive& ar, const std::unique_ptr<T>& ptr, unsigned int) {
			const T * const tx = ptr.get();
			ar << tx;
		}

		template <class Archive, class T> inline void serialize(Archive& ar, std::unique_ptr<T>& ptr, const unsigned int version) {
			boost::serialization::split_free(ar, ptr, version);
		}		
	}
}
