// (C) Averisera Ltd 2014-2020
#pragma once
#include "range.hpp"
#include <string>
#include <type_traits>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>

namespace averisera {
	/** Range [begin, end) */
	template <class T> class NumericalRange: public Range<T> {
		static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");
	public:
		using Range<T>::Range;

		NumericalRange()
			: NumericalRange<T>(T(), T()) {}

		NumericalRange(const Range<T>& other)
			: Range<T>(other) {}

		bool is_adjacent_to(const NumericalRange<T>& other) const {
			return this->end() == other.begin() || this->begin() == other.end();
		}

		bool is_disjoint_with(const NumericalRange<T>& other) const {
			return this->begin() >= other.end() || this->end() <= other.begin();
		}

		static bool all_disjoint(const std::vector<NumericalRange<T>>& ranges, const bool sorted = false) {
			if (sorted) {
				return Inclusion::all_disjoint<true>(ranges);
			} else {
				return Inclusion::all_disjoint<false>(ranges);
			}
		}

		/** Assumes ranges is sorted */
		static bool all_adjacent(const std::vector<NumericalRange<T>>& ranges) {
			if (!ranges.empty()) {
				auto i1 = ranges.begin();
				for (auto i2 = i1 + 1; i2 != ranges.end(); ++i1, ++i2) {
					if (!i1->is_adjacent_to(*i2)) {
						return false;
					}
				}
			}
			return true;
		}

		bool is_contained_by_any(const std::vector<Range<T>>& ranges) const {
			return range_is_contained_by_any(*this, ranges);
		}

		/** Add adjacent range */
		NumericalRange<T> operator+(const NumericalRange<T>& other) const {
			check_that(this->is_adjacent_to(other), "Can only add adjacent ranges");
			if (*this < other) {
				return NumericalRange<T>(this->begin(), other.end());
			} else {
				return NumericalRange<T>(other.begin(), this->end());
			}
		}

		using Range<T>::contains;

		bool contains(const T& val) const {
			return val >= this->begin() && val < this->end();
		}

        using Range<T>::from_string;

		/** Parse "N-M" to [N, M + 1). "N" is parsed to [N, N+1).
		*/
		template <class F> static NumericalRange<T> from_string_open_ended(const char* orig_str, F converter, const T* default_begin, const T* default_end) {
			const NumericalRange<T> range(from_string(orig_str, converter, default_begin, default_end));
			return NumericalRange<T>(range.begin(), range.end() + 1);
		}

		/** Shift the range by delta */
		NumericalRange<T> operator+(T delta) const {
			return NumericalRange<T>(this->begin() + delta, this->end() + delta);
		}
	};

	template <class T> std::ostream& operator<<(std::ostream& os, const NumericalRange<T>& range) {
		os << "[" << +range.begin() << ", " << +range.end() << ")"; // + to print it correctly
		return os;
	}

	// For use in Inclusion algorithms

	template <class T> bool contains(const NumericalRange<T>& a, const NumericalRange<T>& b) {
		return a.contains(b);
	}

	template <class T> bool is_disjoint_with(const NumericalRange<T>& a, const NumericalRange<T>& b) {
		return a.is_disjoint_with(b);
	}
}

// hash function
namespace std {
	template <class T> struct hash<averisera::NumericalRange<T>> {
		size_t operator()(const averisera::NumericalRange<T>& range) const {
			using boost::hash_value;
			using boost::hash_combine;
			size_t seed = hash_value(range.begin());
			hash_combine(seed, hash_value(range.end()));
			return seed;
		}
	};
}
