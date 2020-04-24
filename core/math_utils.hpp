/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_MATH_UTILS_H
#define __AVERISERA_MATH_UTILS_H

#include "data_check_level.hpp"
#include <cstdlib>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/type_index.hpp>

namespace averisera {
	class RNG;

	/** Mathematical utility functions */
	namespace MathUtils {
		/** Sign function: sgn(x > 0) == 1, sgn(0) == 0, sgn(x < 0) == -1. */
		template <typename T> int sgn(T val) {
			return (T(0) < val) - (val < T(0));
		}

		/** Pow function with integer arguments */
		size_t pow(const size_t m, const unsigned int power);
		
		extern const double pi; /**< Pi constant */

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable: 4723)
								// division by 0 inside this function is done on purpose
								// VS demands that we disable the warning for the whole function
#endif
		/** Calculate the logit as a function of probability. */
		template <class Scalar> Scalar prob_to_logit(Scalar p) {
			return log(p / (1.0 - p));
		}
#ifdef _WIN32
#pragma warning(pop)
#endif

		/** Calculate probability as a function of the logit. */
		template <class Scalar> Scalar logit_to_prob(Scalar x) {
			return 1.0 / (exp(- x) + 1);
		}

		/** Calculate the derivative of probability over the logit. */
		template <class Scalar> Scalar logit_to_prob_derivative(Scalar x) {
			// d[1/(exp(-x) + 1)] / dx = exp(-x) / (1 + exp(-x))^2 = 1 / (1 + exp(-x)) / (1 + exp(x)) = 1 / (2 + exp(x) + exp(-x)).
			const Scalar u = exp(x);
			return 1. / (2. + u + 1. / u);
		}

		/** @brief Convert a range of N probabilities to a range of N-1 relative logits.

		A relative logit x[i] = ln(p[i + 1] / p[i]). We assume that probabilities in the range [p_begin, p_end) add up to 1, and none of it is zero.

		@tparam In Iterator over a container of probabilities.
		@tparam Out Iterator over a container of logits.

		@param p_begin Iterator to the beginning of the range of probabilities.
		@param p_end Iterator "one past the end" of the range of probabilities.
		@param x_begin Iterator to the beginning of the range of logits. The range must have space for N-1 logits, given a range of N probabilities.

		@return Iterator "one past the end" of the range of calculated logits, with std::distance(x_begin, [returned value]) + 1 == std::distance(p_begin, p_end).

		@throw std::invalid_argument If p_begin == p_end.
		*/
		template <class In, class Out> Out probabilities_to_relative_logits(const In p_begin, const In p_end, const Out x_begin) {
			if (p_begin == p_end) {
				throw std::invalid_argument("MathUtils::probabilities_to_relative_logits: empty probabilities range");
			}
			In curr_p = p_begin;
			In next_p = p_begin + 1;
			Out curr_x = x_begin;
			auto p0 = *curr_p;
			while (next_p != p_end) {
				assert(curr_p != p_end);
				const auto p1 = *next_p;
				if (p1 != 0 && p0 != 0) {
					*curr_x = log(p1 / p0);
				} else {
					throw std::domain_error("MathUtils::probabilities_to_relative_xs: probabilities cannot be zero");
				}
				p0 = p1;
				++curr_x;
				++curr_p;
				++next_p;
			}
			return curr_x;
		}

		/** Inverse of the function probabilities_to_relative_logits.

		@param p_begin Iterator to the beginning of the range of probabilities. The range must have space for N probabilities, given a range of N-1 logits.
		@return Iterator "one past the end" of the range of calculated probabilities, with std::distance(p_begin, [returned value]) == std::distance(x_begin, x_end) + 1.
		@tparam Out Iterator over a container of probabilities.
		*/
		template <class In, class Out> Out relative_logits_to_probabilities(const In x_begin, const In x_end, const Out p_begin) {
			*p_begin = 1;
			if (x_begin == x_end) {
				return p_begin + 1;
			}
			Out curr_p = p_begin + 1;
			In curr_x = x_begin;
			typename std::remove_reference<decltype(*curr_p)>::type p(1);
			auto sum_p = p;
			while (curr_x != x_end) {
				p *= exp(*curr_x);
				*curr_p = p;
				sum_p += p;
				++curr_x;
				++curr_p;
			}
			const Out p_end = curr_p;
			for (curr_p = p_begin; curr_p != p_end; ++curr_p) {
				(*curr_p) /= sum_p;
			}
			return p_end;
		}

        template <class V, class I> V cum_prod(I begin, I end, V init) {
            return std::accumulate(begin, end, init, [](V prod, V x) {
                    return prod * x;
                });
        }

		/** Safely cast a floating-point value to an integer type.
		@throw std::out_of_range If x cannot fit in the range of type I
		*/
		template <class T, class S> T safe_cast(S x) {
			try {
				return boost::numeric_cast<T>(x);
			} catch (std::exception& e) {
				throw std::out_of_range(boost::str(boost::format("Utils: safe_cast of %s from type %s to %s failed due to: %s")
					% boost::lexical_cast<std::string>(x)
					% boost::typeindex::type_id<S>().pretty_name()
					% boost::typeindex::type_id<T>().pretty_name()
					% e.what()
					));
			}
		}

        /** Used to partially specialise more complex casts which do not do any safety checks. */
        template <class T> struct static_caster {
            template <class S> static T apply(S x) {
                return static_cast<T>(x);
            }
        };

		/** @tparam V Vector type with .size() and [] operator */
		template <class V> bool is_any_nan(const V& v) {
			const size_t s = static_cast<size_t>(v.size());
			for (size_t i = 0; i < s; ++i) {
				if (std::isnan(v[i])) {
					return true;
				}
			}
			return false;
		}

		/** @tparam V Vector type with .size() and [] operator */
		template <class V> bool are_all_finite(const V& v) {
			const size_t s = static_cast<size_t>(v.size());
			for (size_t i = 0; i < s; ++i) {
				if (!std::isfinite(v[i])) {
					return false;
				}
			}
			return true;
		}
		
		/** Check if vector data passes given level of checks 
		@tparam V Vector type with .size() and [] operator */
		template <class V> bool check_data(const V& v, DataCheckLevel check_level) {
			switch (check_level) {
			case DataCheckLevel::ANY:
				return true;
			case DataCheckLevel::NOT_NAN:
				return !is_any_nan(v);
			case DataCheckLevel::FINITE:
				return are_all_finite(v);
			default:
				throw std::logic_error("MathUtils: unknown check level");
			}
		}

   //     template <class I, class F> typename std::enable_if<std::is_floating_point<F>::value && std::is_integral<I>::value, I>::type safe_cast(F x) {
   //         // See http://stackoverflow.com/questions/526070/handling-overflow-when-casting-doubles-to-integers-in-c
   //         constexpr bool hi_check = std::numeric_limits<I>::max() <= std::numeric_limits<F>::max();
   //         constexpr bool lo_check = std::numeric_limits<I>::min() >= -std::numeric_limits<F>::max();
   //         if ((hi_check && static_cast<F>(std::numeric_limits<I>::max()) <= x)
   //             ||
   //             (lo_check && static_cast<F>(std::numeric_limits<I>::min()) > x)) {
   //             throw std::out_of_range("Overflow detected when casting"); // Use Boost to print a nice error message with types and value specified
   //         } else {
   //             return static_cast<I>(x);
   //         }
   //     }

   //     /** Safely cast an integer value I2 to another integer type I. Either both types are signed or both are unsigned.
   //       @throw std::out_of_range If x cannot fit in the range of type I
   //     */
   //     template <class I, class I2> typename std::enable_if<std::is_integral<I>::value && std::is_integral<I2>::value && (std::numeric_limits<I>::is_signed == std::numeric_limits<I2>::is_signed) && (!std::is_same<I, I2>::value), I>::type safe_cast(I2 x) {
   //         // See http://stackoverflow.com/questions/526070/handling-overflow-when-casting-doubles-to-integers-in-c
   //         constexpr bool hi_check = std::numeric_limits<I>::max() < std::numeric_limits<I2>::max();
   //         constexpr bool lo_check = std::numeric_limits<I>::min() > std::numeric_limits<I2>::min();
   //         if ((hi_check && static_cast<I2>(std::numeric_limits<I>::max()) <= x)
   //             ||
   //             (lo_check && static_cast<I2>(std::numeric_limits<I>::min()) > x)) {
   //             throw std::out_of_range("Overflow detected when casting"); // Use Boost to print a nice error message with types and value specified
   //         } else {
   //             return static_cast<I>(x);
   //         }
   //     }

   //     /** Safely cast a signed integer value I2 to another, unsigned integer type I.
   //       @throw std::out_of_range If x cannot fit in the range of type I
   //     */
   //     template <class I, class I2> typename std::enable_if<std::is_integral<I>::value && std::is_integral<I2>::value && (!std::numeric_limits<I>::is_signed) && std::numeric_limits<I2>::is_signed && (!std::is_same<I, I2>::value), I>::type safe_cast(I2 x) {
   //         // See http://stackoverflow.com/questions/526070/handling-overflow-when-casting-doubles-to-integers-in-c
   //         static_assert(std::numeric_limits<I>::min() == 0, "Min. value of unsigned integer type assumed to be 0");
   //         static_assert(std::numeric_limits<I2>::min() <= 0, "Min. value of any integer type assumed to be <= 0");
   //         constexpr bool hi_check = std::numeric_limits<I>::max() < std::numeric_limits<typename std::make_unsigned<I2>::type>::max();
   //         if ((hi_check && static_cast<I2>(std::numeric_limits<I>::max()) <= x)
   //             ||
   //             (static_cast<I2>(std::numeric_limits<I>::min()) > x)) {
   //             throw std::out_of_range("Overflow detected when casting"); // Use Boost to print a nice error message with types and value specified
   //         } else {
   //             return static_cast<I>(x);
   //         }
   //     }

   //     /** Safely cast an unsigned integer value I2 to another, signed integer type I.
   //       @throw std::out_of_range If x cannot fit in the range of type I
   //     */
   //     template <class I, class I2> typename std::enable_if<std::is_integral<I>::value && std::is_integral<I2>::value && std::numeric_limits<I>::is_signed && (!std::numeric_limits<I2>::is_signed) && (!std::is_same<I, I2>::value), I>::type safe_cast(I2 x) {
   //         // See http://stackoverflow.com/questions/526070/handling-overflow-when-casting-doubles-to-integers-in-c
   //         static_assert(std::numeric_limits<I2>::min() == 0, "Min. value of unsigned integer type assumed to be 0");
   //         static_assert(std::numeric_limits<I>::min() <= 0, "Min. value of any integer type assumed to be <= 0");
			//constexpr bool hi_check = (std::numeric_limits<typename std::make_unsigned<I>::type>::max() / 2) <= std::numeric_limits<I2>::max();
   //         if (hi_check && static_cast<I2>(std::numeric_limits<I>::max()) <= x) {
   //             throw std::out_of_range("Overflow detected when casting"); // Use Boost to print a nice error message with types and value specified
   //         } else {
   //             return static_cast<I>(x);
   //         }
   //     }

   //     /** Cast an integer value to a floating-point type. For completeness. Does not throw.
   //     */
   //     template <class F, class I> typename std::enable_if<std::is_floating_point<F>::value && std::is_integral<I>::value, F>::type safe_cast(I x) {
   //         return static_cast<F>(x);
   //     }

   //     /** No-op for no type conversion. */
   //     template <class T1, class T2> typename std::enable_if<std::is_same<T1, T2>::value, T1>::type safe_cast(T2 x) {
   //         return x;
   //     }

		/** Randomly round x to either c := ceil(x) or f := floor(x). 
		Return c with probability (x - f) or f with probability (c - x).
		If x is integer, always returns x.
		*/
		double random_round(double x, RNG& rng);

		/** Solve equation a*x^2 + b*x + c == 0.
		@param[out] x1 First root
		@param[out] x2 Second root (set only if a != 0 and  Delta > 0)
		@return Number of roots
		*/
		int solve_quadratic(double a, double b, double c, double& x1, double& x2);

		/** Find a the iterator to the minimum element of [begin, end) range, choosing randomly the element in case there is more than 1 with the minimum value 
          @tparam I iterator type
          @tparam R RNG type (has to be templatized to avoid compilation problems)
		*/
		template <class I, class R> I min_element_randomised(I begin, const I end, R& rng) {
			if (begin == end) {
				return end;
			}
			I result = begin;
			size_t nbr_values = 1;
			++begin;
			while (begin != end) {
				if (*begin < *result) {
					// new minimum found
					nbr_values = 1;
					result = begin;
				} else if (*begin == *result) {
					++nbr_values;
					// switch to new element with probability 1 / nbr_values
					const double p = 1 / static_cast<double>(nbr_values);
					if (rng.flip(p)) {
						result = begin;
					}
				}
				++begin;
			}
			return result;
		}

		/** Find a the iterator to the maximum element of [begin, end) range, choosing randomly the element in case there is more than 1 with the maximum value
          @tparam I iterator type
          @tparam R RNG type (has to be templatized to avoid compilation problems)
		*/
		template <class I, class R> I max_element_randomised(I begin, const I end, R& rng) {
			if (begin == end) {
				return end;
			}
			I result = begin;
			size_t nbr_values = 1;
			++begin;
			while (begin != end) {
				if (*begin > *result) {
					// new minimum found
					nbr_values = 1;
					result = begin;
				} else if (*begin == *result) {
					++nbr_values;
					// switch to new element with probability 1 / nbr_values
					const double p = 1 / static_cast<double>(nbr_values);
					if (rng.flip(p)) {
						result = begin;
					}
				}
				++begin;
			}
			return result;
		}
	}
}

#endif
