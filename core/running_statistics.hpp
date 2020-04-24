/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_RUNNING_STATISTICS_H
#define __AVERISERA_RUNNING_STATISTICS_H

#include "math_utils.hpp"
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <vector>

namespace averisera {
	/**
	Accumulates running statistics from a sample
	@tparam T scalar floating point type for calculations
	*/
	template <class T = double> class RunningStatistics {
	public:
		RunningStatistics();

		// Add new sample x.
		void add(T x);

		// Add new sample if it's finite. Return whether it was added or not.
		bool add_if_finite(T x);

		/** Add new sample if it's not NaN. Return whether it was added or not. */
		bool add_if_not_nan(T x);

		// Add new sample x as a sum of vector elements
		void add(const std::vector<T>& elems);

		// Add new sample x as a sum of vector elements, if it is finite. Return whether it was added or not.
		bool add_if_finite(const std::vector<T>& elems);

		// Minimal value
		T min() const { return _min; }

		// Maximum value
		T max() const { return _max; }

		// Mean
		T mean() const { return _m1; }

		/** Sample variance, unbiased (divided by N - 1) */
		T variance() const;

		/** Sample standard deviation */
		T standard_deviation() const;

		/** Skewness */
		T skewness() const;

		/** Excess curtosis */
		T kurtosis() const;

        typedef uint64_t counter_t; /**< Type used to count the samples */

		/** Number of accumulated samples */
		counter_t nbr_samples() const { return _cnt; }

        /** Multiply the statistics by a const factor */
        template <class S> RunningStatistics<T> operator*(S x) const {
            RunningStatistics<T> r(*this);
            if (x >= 0.0) {
                r._min *= x;
                r._max *= x;
            } else {
                r._min = _max * x;
                r._max = _min * x;
            }
            r._m1 *= x;
            r._m2 *= std::pow(x, 2);
            r._m3 *= std::pow(x, 3);
            r._m4 *= std::pow(x, 4);
            return r;
        }

		/** Shift the statistics by a const term */
		template <class S> RunningStatistics<T> operator+(S x) const {
			RunningStatistics<T> r(*this);
			r._min += x;
			r._max += x;
			r._m1 += x;
			return r;
		}
	private:
		T _min;
		T _max;
		T _m1;
		T _m2;
		T _m3;
		T _m4;
		counter_t _cnt; // 32 bits is too little for this

        T fcnt() const {
			return MathUtils::static_caster<T>::apply(_cnt);
        }
	};

    template <class S, class T> RunningStatistics<T> operator*(S x, const RunningStatistics<T>& rs) {
        return rs * x;
    }
}


#endif // __AVERISERA_RUNNING_STATISTICS_H
