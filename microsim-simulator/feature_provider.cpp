#include "feature_provider.hpp"
#include "feature.hpp"

namespace averisera {
    namespace microsim {
        // Commonly used instance of this template
        template <> class FeatureProvider<Feature>;
    }
}