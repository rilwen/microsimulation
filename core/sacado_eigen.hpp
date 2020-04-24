/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_SACADO_EIGEN_H
#define __AVERISERA_SACADO_EIGEN_H

#include "sacado_scalar.hpp"

// Declarations which allow to use Sacado automatic differentiation library together with Eigen for simple matrix algebra.
// Warning: complicated operations like matrix inversion or SVD will probably not work!

namespace Sacado {
	
#ifdef _WIN64
	// Add specialisation for __int64 type for 64 bit build
#define SACADO_BUILTIN_SPECIALIZATION(t,NAME)             \
	template <> struct ScalarType< t > {                    \
	typedef t type;                                       \
};                                                      \
	template <> struct ValueType< t > {                     \
	typedef t type;                                       \
};                                                      \
	template <> struct IsADType< t > {                      \
	static const bool value = false;                      \
};                                                      \
	template <> struct IsScalarType< t > {                  \
	static const bool value = true;                       \
};                                                      \
	template <> struct Value< t > {                         \
	KOKKOS_INLINE_FUNCTION                                \
	static const t& eval(const t& x) { return x; }        \
};                                                      \
	template <> struct ScalarValue< t > {                   \
	KOKKOS_INLINE_FUNCTION                                \
	static const t& eval(const t& x) { return x; }        \
};                                                      \
	template <> struct StringName< t > {                    \
	KOKKOS_INLINE_FUNCTION                                \
	static std::string eval() { return NAME; }            \
};                                                      \
	template <> struct IsEqual< t > {                       \
	KOKKOS_INLINE_FUNCTION                                \
	static bool eval(const t& x, const t& y) {            \
	return x == y; }                                    \
};                                                      \
	template <> struct IsStaticallySized< t > {             \
	static const bool value = true;                       \
};

	SACADO_BUILTIN_SPECIALIZATION(__int64, "__int64")

#undef SACADO_BUILTIN_SPECIALIZATION
#endif // _WIN64
}



#include <Eigen/Core>

namespace Eigen {
	using averisera::adouble;

	template<> struct NumTraits<adouble>
		: NumTraits<double> // permits to get the epsilon, dummy_precision, lowest, highest functions
		{
			typedef adouble Real;
			typedef adouble NonInteger;
			typedef adouble Nested;
			enum {
				IsComplex = 0,
				IsInteger = 0,
				IsSigned = 1,
				RequireInitialization = 1,
				ReadCost = 1,
				AddCost = 3,
				MulCost = 3
			};
		};

		typedef Matrix<adouble, Dynamic, Dynamic> MatrixXa;
		typedef Matrix<adouble, Dynamic, 1> VectorXa;
		typedef Matrix<adouble, 2, 1> Vector2a;
		typedef Matrix<adouble, 3, 1> Vector3a;
		typedef Matrix<adouble, 4, 1> Vector4a;
}


#endif