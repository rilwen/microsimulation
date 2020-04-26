// (C) Averisera Ltd 2014-2020
#pragma once
#include "data_exception.hpp"
#include "inclusion.hpp"
#include "preconditions.hpp"
#include <cassert>
#include <iosfwd>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>

namespace averisera {
	
	/** Range a-b -- an ordered pair a, b with a <= b */
	template <class T> class Range {
	public:
		Range()
			: Range<T>(T(), T()) {}

		typedef T value_type;
		/** begin <= end */
		Range(T begin, T end)
			: begin_(begin), end_(end) {
			check_that(begin <= end, "Range: begin <= end");
		}

		Range(std::pair<T, T> values)
			: Range(values.first, values.second) {}

		T begin() const { return begin_; }

		T end() const { return end_; }

		bool operator==(const Range<T>& other) const {
			return begin_ == other.begin_ && end_ == other.end_;
		}

		bool operator!=(const Range<T>& other) const {
			return !(*this == other);
		}

		/** Lexical comparison, first begin then end */
		bool operator<(const Range<T>& other) const {
			if (begin() != other.begin()) {
				return begin() < other.begin();
			} else {
				return end() < other.end();
			}
		}

		bool operator<=(const Range<T>& other) const {
			return (*this < other) || (*this == other);
		}

		bool is_disjoint_with(const Range<T>& other) const {
			return begin() > other.end() || end() < other.begin();
		}

		bool is_disjoint_with_all(const std::vector<Range<T>>& others) const {
			return Inclusion::is_disjoint_with_all(*this, others);
		}

		static bool all_disjoint(const std::vector<Range<T>>& ranges, const bool sorted = false) {
			if (sorted) {
				return Inclusion::all_disjoint<true>(ranges);
			} else {
				return Inclusion::all_disjoint<false>(ranges);
			}
		}

		/** Check other in this */
		bool contains(const Range<T>& other) const {
			return begin() <= other.begin() && end() >= other.end();
		}

		InclusionRelation inclusion(const Range<T>& b) const {
			return Inclusion::inclusion(*this, b);
		}

		bool is_contained_by_any(const std::vector<Range<T>>& ranges) const {
			return Inclusion::is_contained_by_any(*this, ranges);
		}		

		/** Convert a string N-M or N into a range. String "N" will be converted into [N, N]
		@param converter Functor which takes a std::string and returns a T value or throws DataException if unable to convert
		@param default_begin Default value for begin if N is empty in an N-M range
		@param default_end Default value for end if M is empty in an N-M range
		@throw DataException If string is empty or only whitespace
		*/
		template <class F> static Range<T> from_string(const char* orig_str, F converter, const T* default_begin, const T* default_end) {
			std::vector<std::string> groups;
			std::string str(orig_str);
			boost::erase_all(str, " ");
			check_that<DataException>(!str.empty(), "Range: string is empty or only whitespace");
			boost::split(groups, str, boost::is_any_of("-"));
			T begin;
			T end;
			switch (groups.size()) {
			case 1:
				begin = converter(groups[0]);
				end = begin;
				break;
			case 2:
				if (groups[0].empty()) {
					if (default_begin) {
						begin = *default_begin;
					} else {
						throw DataException("Range: no default beginning value for range");
					}
				} else {
					begin = converter(groups[0]);
				}
				if (groups[1].empty()) {
					if (default_end) {
						end = *default_end;
					} else {
						throw DataException("Range: no default end value for range");
					}
				} else {
					end = converter(groups[1]);
				}
				break;
			default:
				throw DataException(boost::str(boost::format("Range: cannot convert name \"%s\" to a range") % orig_str));
			}
			if (begin > end) {
				throw DataException(boost::str(boost::format("Range: group range \"%s\" out of order") % orig_str));
			}
			return Range<T>(begin, end);
		}
	private:
		T begin_;
		T end_;
	};

	template <class T> std::ostream& operator<<(std::ostream& os, const Range<T>& range) {
		os << "[" << range.begin() << ", " << range.end() << "]";
		return os;
	}

	template <> inline std::ostream& operator<<(std::ostream& os, const Range<uint8_t>& range) {
		os << "[" << static_cast<int>(range.begin()) << ", " << static_cast<int>(range.end()) << "]";
		return os;
	}

	template <> inline std::ostream& operator<<(std::ostream& os, const Range<int8_t>& range) {
		os << "[" << static_cast<int>(range.begin()) << ", " << static_cast<int>(range.end()) << "]";
		return os;
	}

	// For use in Inclusion algorithms

	template <class T> bool contains(const Range<T>& a, const Range<T>& b) {
		return a.contains(b);
	}

	template <class T> bool is_disjoint_with(const Range<T>& a, const Range<T>& b) {
		return a.is_disjoint_with(b);
	}
}

// hash function
namespace std {
	template <class T> struct hash<averisera::Range<T>> {		
		size_t operator()(const averisera::Range<T>& range) const {
			using boost::hash_value;
			using boost::hash_combine;
			size_t seed = hash_value(range.begin());
			hash_combine(seed, hash_value(range.end()));
			return seed;
		}
	};
}
