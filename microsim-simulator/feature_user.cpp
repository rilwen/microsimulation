#include "feature_user.hpp"
#include "feature.hpp"
#include <iterator>
#include <string>

namespace averisera {
    namespace microsim {
		template <class F> FeatureUser<F>::~FeatureUser() {}

		template <class F> typename FeatureUser<F>::feature_set_t FeatureUser<F>::combine(const feature_set_t& first, const feature_set_t& second) {
			feature_set_t result;
			std::set_union(first.begin(), first.end(), second.begin(), second.end(), std::inserter(result, result.begin()));
			return result;
		}

        // Commonly used instance of this template
        template class FeatureUser<Feature>;
		template class FeatureUser<std::string>; // for testing
    }
}