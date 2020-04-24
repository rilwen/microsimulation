/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_SACADO_SCALAR_H
#define __AVERISERA_SACADO_SCALAR_H

#include "math_utils.hpp"
#include <cmath>

#ifdef _MSC_VER
#pragma warning(disable : 4396 4099)
#endif // _MSC_VER
#include <Sacado.hpp>
#ifdef _MSC_VER
#pragma warning(default : 4396 4099)
#endif // _MSC_VER


namespace averisera {
	typedef Sacado::Fad::DFad<double> adouble;
	typedef Sacado::Fad::DFad<adouble> addouble;

	template <unsigned int NestingLevel> struct NestedADouble;

	/** Nested automatically differentiable double type.

	@tparam L Nesting level. NestedADouble<L> tracks derivatives up to and including (L+1)-th order.
	*/
	template <unsigned int L> struct NestedADouble {
		typedef typename NestedADouble<L - 1>::ad_type value_type;
		typedef Sacado::Fad::DFad<value_type> ad_type;		

		static ad_type from_double(const unsigned int dim, const unsigned int idx, double value) {
			return ad_type(dim, idx, NestedADouble<L - 1>::from_double(dim, idx, value));
		}

		static double to_double(const ad_type& value) {
			return NestedADouble<L - 1>::to_double(value.val());
		}
	};

	template <> struct NestedADouble<0> {
		typedef double value_type;
		typedef Sacado::Fad::DFad<value_type> ad_type;
		static ad_type from_double(const unsigned int dim, const unsigned int idx, double value) {
			return ad_type(dim, idx, value);
		}

		static double to_double(const ad_type& value) {
			return value.val();
		}
	};

	// running_index: Increased after conversion
	template <unsigned int NestingLevel> typename NestedADouble<NestingLevel>::ad_type from_double(const unsigned int total_number, unsigned int& running_index, double value) {
		if (running_index < total_number) {
			typename NestedADouble<NestingLevel>::ad_type result = NestedADouble<NestingLevel>::from_double(total_number, running_index, value);
			++running_index;
			return result;
		} else {
			return adouble(value);
		}
	}	

	// If running_index < total_number, add the created adouble to tracking, otherwise not.
	inline adouble double2adouble(const unsigned int total_number, unsigned int& running_index, double value) {
		return from_double<0>(total_number, running_index, value);
	}

    namespace MathUtils {
        template <> struct static_caster<adouble> {
            template <class S> static adouble apply(S x) {
                return adouble(static_cast<double>(x));
            }
        };
    }
}

namespace std {
    inline bool isnan(const averisera::adouble& x) {
        return std::isnan(x.val());
    }

    inline bool isinf(const averisera::adouble& x) {
        return std::isinf(x.val());
    }

    inline bool isfinite(const averisera::adouble& x) {
        return std::isfinite(x.val());
    }

    inline bool signbit(const averisera::adouble& x) {
        return std::signbit(x.val());
    }    
}

#endif // __AVERISERA_SACADO_SCALAR_H
