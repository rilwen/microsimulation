#ifndef __AVERISERA_TESTING_ASSERTIONS_HPP
#define __AVERISERA_TESTING_ASSERTIONS_HPP

#include <cassert>
#include <cmath>
#include <gtest/gtest.h>

namespace averisera {
    namespace testing {
        template <class T> bool is_near(const T expected, const T actual, const T delta) {
            assert(delta >= 0);
            return std::abs(actual - expected) <= delta;
        }

		template <class V1, class V2, class T> void ASSERT_ALL_NEAR(const V1& expected, const V2& actual, const T delta, const char* test_description) {
			const auto n = expected.size();
			ASSERT_EQ(n, actual.size()) << "Vector comparison " << test_description << " failed: unequal vector sizes";			
			for (decltype(expected.size()) i = 0; i < n; ++i) {
				ASSERT_NEAR(expected[i], actual[i], delta) << "Vector comparison " << test_description << "failed: absolute value difference too large at position " << i;
			}
		}
    }
}

#endif // __AVERISERA_TESTING_ASSERTIONS_HPP
