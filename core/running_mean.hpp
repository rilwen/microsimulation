#pragma once
#include <cstdint>
#include <vector>

namespace averisera {
	/** Accumulate a mean from a sample */
	template <class T> class RunningMean {
	public:
		RunningMean();

		/** Add new sample x. */
		void add(T x);

		/** Add new sample x assuming it's finite. */
		void add_finite(T x);

		/** Add new sample if it's finite. Return whether it was added or not. */
		bool add_if_finite(T x);

		/** Add new sample if it's not NaN. Return whether it was added or not. */
		bool add_if_not_nan(T x);

		/** Add new sample x as a sum of vector elements */
		void add(const std::vector<T>& elems);

		/** Add new sample x as a sum of vector elements, if it is finite. Return whether it was added or not. */
		bool add_if_finite(const std::vector<T>& elems);

		/** Mean */
		T mean() const;

		typedef uint64_t counter_t; /**< We need a large range */

		/** Number of accumulated samples */
		counter_t nbr_samples() const { return _cnt; }

		/** Reset to initial state */
		void reset();
	protected:
		T fcnt() const {
			return static_cast<T>(nbr_samples());
		}
	private:
		T _m1;
		counter_t _cnt; 
	};
}
