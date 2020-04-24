#ifndef __AVERISERA_KAHAN_SUMMATION_H
#define __AVERISERA_KAHAN_SUMMATION_H

#include <type_traits>
#include <cmath>
#include <iosfwd>

namespace averisera {

	// Template class which sums numbers more accurately. Uses operator overloading to provide natural syntax.
	// T is a floating point type
	template <class T> class KahanSummation {
		static_assert(std::is_floating_point<T>::value, "T must be a floating point type");
	public:
		// Construct with initial value, which defaults to zero.
		KahanSummation(T init = 0)
			: _sum(init), _c(0)
		{}

		// Reset value to val, clearing the auxiliary values.
		KahanSummation<T>& operator=(T val) {
			_sum = val;
			_c = 0;
			return *this;
		}

		// Add another value.
		KahanSummation<T>& operator+=(T val) {
			switch (std::fpclassify(val)) {
			case FP_INFINITE:
			case FP_NAN:
				// Enforce that identities like inf - inf == NaN are respected
				_sum += val;
				_c = 0;
				break;
			default:
				const T y = val - _c;
				const T t = _sum + y;
				_c = (t - _sum) - y;
				_sum = t;
			}
			return *this;
		}

		// Convert to base floating point type T
		operator T() const { return _sum; }
	private:
		T _sum;
		T _c;
	};

	template <class T> std::ostream& operator<<(std::ostream& out, const KahanSummation<T>& kh) {
		out << static_cast<double>(kh);
		return out;
	}
}

#endif
