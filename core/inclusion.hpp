// (C) Averisera Ltd 2014-2020
#pragma once
#include <algorithm>
#include <iosfwd>
#include <set>
#include <unordered_set>
#include "log.hpp"
#include "stl_utils.hpp"

namespace averisera {
	/** Constants describing the inclusion relation between ranges/sets A and B */
	enum class InclusionRelation {
		CONTAINS = 0, /**< A contains B */
		IS_CONTAINED_BY, /**< A is contained by B */
		EQUALS, /** A == B */
		DISJOINT, /** A is disjoint with B */
		UNDEFINED /** Neither is contained in the other */
	};

	std::ostream& operator<<(std::ostream& os, InclusionRelation rel);

	namespace Inclusion {
		/** Check if b is in a */
		template <class T> bool contains(const T& a, const T& b);

		template <class T> bool contains(const std::unordered_set<T>& a, const std::unordered_set<T>& b) {
			return std::all_of(b.begin(), b.end(), [&a](const T& t) {
                    return a.find(t) != a.end();
                });
		}

		template <class T> bool contains(const std::set<T>& a, const std::set<T>& b) {
			return std::includes(a.begin(), a.end(), b.begin(), b.end());
		}

		/** Check if a is in b */
		template <class T> bool is_contained_by(const T& a, const T& b) {
			return contains(b, a);
		}

		template <class R> bool is_contained_by_any(const R& elem, const std::vector<R>& v) {
			if (!v.empty()) {
				for (const auto& r : v) {
					if (contains(r, elem)) {
						return true;
					}
				}
			}
			return false;
		}

		/** a and b have no common elements */
		template <class T> bool is_disjoint_with(const T& a, const T& b);

		template <class T> bool is_disjoint_with(const std::unordered_set<T>& a, const std::unordered_set<T>& b) {
			for (const T& a_elem : a) {
				if (b.find(a_elem) != b.end()) {
					return false;
				}
			}
			return true;
		}

		template <class T> bool is_disjoint_with(const std::set<T>& a, const std::set<T>& b) {
			for (const T& a_elem : a) {
				if (b.count(a_elem)) {
					return false;
				}
			}
			return true;
		}

		template <class T> bool is_disjoint_with_all(const T& t, const std::vector<T>& v) {
			return std::all_of(v.begin(), v.end(), [&t](const T& t2) { return is_disjoint_with(t, t2); });
		}

		template <class T> InclusionRelation inclusion(const T& a, const T& b) {
			if (a == b) {
				return InclusionRelation::EQUALS;
			} else {
				if (contains(a, b)) {
					return InclusionRelation::CONTAINS;
				} else if (contains(b, a)) {
					return InclusionRelation::IS_CONTAINED_BY;
				} else if (is_disjoint_with(a, b)) {
					return InclusionRelation::DISJOINT;
				} else {
					return InclusionRelation::UNDEFINED;
				}
			}
		}

		template <bool Sorted, class R> bool all_disjoint(const std::vector<R>& v) {
			for (auto i1 = v.begin(); i1 != v.end(); ++i1) {
				if (Sorted) {
					const auto i2 = i1 + 1;
					if (i2 != v.end()) {
						if (!is_disjoint_with(*i1, *i2)) {
							return false;
						}
					}
				} else {
					for (auto i2 = i1 + 1; i2 != v.end(); ++i2) {
						if (!is_disjoint_with(*i1, *i2)) {
							return false;
						}
					}
				}
			}
			return true;
		}

		/** Finds the inclusion relation of two vectors of disjoint ranges/sets.
		EQUALS: v1 == v2
		CONTAINS: every range in v2 is contained by a range in v1
		IS_CONTAINED_BY: every range in v1 is contained by a range in v2
		DISJOINT: all ranges in v1 are disjoint with all ranges in v2
		UNDEFINED: none of the above
		Assumes that v1 and v2 are all disjoint */
		template <class R> InclusionRelation disjoint_elements_inclusion(const std::vector<R>& v1, const std::vector<R>& v2) {
			assert(Inclusion::all_disjoint<false>(v1));
			assert(Inclusion::all_disjoint<false>(v2));
			if (v1.size() == v2.size() && std::all_of(v1.begin(), v1.end(), [&v2](const R& r1) {
				return std::any_of(v2.begin(), v2.end(), [&r1](const R& r2) {
					return r1 == r2;
				});
			})) {
				return InclusionRelation::EQUALS;
			}
			if (std::all_of(v2.begin(), v2.end(), [&v1](const R& r) {
                        const bool result = is_contained_by_any(r, v1);
                        //LOG_TRACE() << r << " contained by any in " << v1 << ": " << result;
                        return result;
                    })) {
				return InclusionRelation::CONTAINS;
			}
			if (std::all_of(v1.begin(), v1.end(), [&v2](const R& r) { return is_contained_by_any(r, v2); })) {
				return InclusionRelation::IS_CONTAINED_BY;
			}
			if (std::all_of(v1.begin(), v1.end(), [&v2](const R& r) { return is_disjoint_with_all(r, v2); })) {
				return InclusionRelation::DISJOINT;
			}
			return InclusionRelation::UNDEFINED;
		}
	}
}
