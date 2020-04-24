/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_PRECONDITIONS_H
#define __AVERISERA_PRECONDITIONS_H

#include <algorithm>
#include <initializer_list>
#include <stdexcept>
#include <vector>
#include <memory>
#include <boost/format.hpp>

namespace averisera {

	// Helper functions to test function preconditions.

	// Check that ptr is not null
	// ptr: Raw pointer
	// msg: Message to be displayed if condition not met.
	template <class T> void check_not_null(T* ptr, const char* msg = 0) {
		if (ptr == 0) {
			if (msg != 0) {
				throw std::domain_error(boost::str(boost::format("Null pointer: %s") % msg));
			} else {
				throw std::domain_error("Null pointer");
			}
		}
	}

	// Check that shared pointer is not null.
	// ptr: Shared pointer
	// msg: Message to be displayed if condition not met.
	template <class T> void check_not_null(std::shared_ptr<T> ptr, const char* msg = 0) {
		check_not_null(ptr.get(), msg);
	}

	// Check that unique pointer is not null.
	// ptr: Unique pointer
	// msg: Message to be displayed if condition not met.
	template <class T> void check_not_null(const std::unique_ptr<T>& ptr, const char* msg = 0) {
		check_not_null(ptr.get(), msg);
	}

    /** Check if any pointer in the range is null */
    template <class Ptr, class I> void check_all_not_null(I begin, I end, const char* msg = 0) {
        if (std::any_of(begin, end, [](const Ptr& ptr) { return !ptr; })) {
            if (msg) {
                throw std::domain_error(std::string("Null pointer in range: ") + msg);
            } else {
                throw std::domain_error("Null pointer in range");
            }
        }
    }

    /** Check if any pointer in the vector is null */
    template <class Ptr> void check_all_not_null(const std::vector<Ptr>& ptrs, const char* msg = 0) {
        check_all_not_null<Ptr>(ptrs.begin(), ptrs.end(), msg);
    }

    /** Check if any pointer in the initializer list is null */
    template <class Ptr> void check_all_not_null(std::initializer_list<Ptr> ptrs, const char* msg = 0) {
        check_all_not_null<Ptr>(ptrs.begin(), ptrs.end(), msg);
    }

	// Check if idx < size
	inline void check_index(unsigned int idx, unsigned int size) {
		if (idx >= size) {
			throw std::domain_error("Index out of bounds");
		}
	}

	// Check if two values are exactly equal
	// expected: Expected value
	// actual: Actual value
	// msg: Message to be displayed if condition not met.
	template <class T1, class T2, class E=std::domain_error> void check_equals(const T1& expected, const T2& actual, const char* msg = 0) {
		if (!(expected == static_cast<T1>(actual))) {
			if (msg) {
				throw E(boost::str(boost::format("Expected %g but actual %g: %s") % expected % actual % msg));
			} else {
				throw E(boost::str(boost::format("Expected %g but actual %g") % expected % actual));
			}
		}
	}

	// Check if two values are not equal
	// expected: Expected value
	// actual: Actual value
	// msg: Message to be displayed if condition not met.
	template <class T1, class T2, class E = std::domain_error> void check_not_equals(const T1& expected, const T2& actual, const char* msg = 0) {
		if (!(expected != static_cast<T1>(actual))) {
			if (msg) {
				throw E(boost::str(boost::format("Expected values to be different but both %g: %s") % expected % msg));
			} else {
				throw E(boost::str(boost::format("Expected values to be different but both %g") % expected % actual));
			}
		}
	}

	/** Check if left is greater than right */
	template <class T1, class T2, class E = std::domain_error> void check_greater(const T1& left, const T2& right, const char* msg = 0) {
		if (!(left > right)) {
			if (msg) {
				throw E(boost::str(boost::format("Expected %g to be greater than %g: %s") % left % right % msg));
			} else {
				throw E(boost::str(boost::format("Expected %g to be greater than %g") % left % right));
			}
		}
	}

	/** Check if left is greater or equal to right */
	template <class T1, class T2, class E = std::domain_error> void check_greater_or_equal(const T1& left, const T2& right, const char* msg = 0) {
		if (!(left >= right)) {
			if (msg) {
				throw E(boost::str(boost::format("Expected %g to be greater or equal %g: %s") % left % right % msg));
			} else {
				throw E(boost::str(boost::format("Expected %g to be greater or equal %g") % left % right));
			}
		}
	}

	/** Check if left is less than right */
	template <class T1, class T2, class E = std::domain_error> void check_less(const T1& left, const T2& right, const char* msg = 0) {
		if (!(left < right)) {
			if (msg) {
				throw E(boost::str(boost::format("Expected %g to be less than %g: %s") % left % right % msg));
			} else {
				throw E(boost::str(boost::format("Expected %g to be less than %g") % left % right));
			}
		}
	}

	/** Check if left is less or equal to right */
	template <class T1, class T2, class E = std::domain_error> void check_less_or_equal(const T1& left, const T2& right, const char* msg = 0) {
		if (!(left <= right)) {
			if (msg) {
				throw E(boost::str(boost::format("Expected %g to be less or equal %g: %s") % left % right % msg));
			} else {
				throw E(boost::str(boost::format("Expected %g to be less or equal %g") % left % right));
			}
		}
	}

	/** Check that condition is true and throw std::domain_error if it is not the case
	@param condition_description Description of the tested condition */
	template <class E=std::domain_error> void check_that(bool condition, const char* condition_description = 0) {
		if (!condition) {
			if (!condition_description) {
				throw E("Condition not met");
			} else {
				throw E(boost::str(boost::format("Condition \"%s\" not met") % condition_description));
			}
		}
	}

	// Check that predicate is true to every element of [begin, end)
	template <class Iter, class Predicate> void check_that(Iter begin, const Iter end, Predicate predicate, const char* msg = 0) {
		while (begin != end) {
			check_that(predicate(*begin), msg);
			++begin;
		}
	}
	 
	// Check that sum of elements is as expected, with tolerance
	// expected: Expected value of the sum
	// elements: Vector of elements
	// tolerance: Absolute tolerance
	// msg: Message to be displayed if condition not met.
	void check_sum(double expected, const std::vector<double>& elements, double tolerance, const char* msg = 0);

	// Check that probs array defines a probability distribution
	// probs: Vector object with .size() and [] operator
	// tolerance: tolerance for the sum of probabilities
	// msg: Message to be displayed if condition not met.
	void check_distribution(const std::vector<double>& probs, double tolerance = 1E-12, const char* msg = 0);


}

#endif
