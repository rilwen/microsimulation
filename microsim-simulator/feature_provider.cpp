// (C) Averisera Ltd 2014-2020
#include "feature_provider.hpp"
#include "feature.hpp"

namespace averisera {
    namespace microsim {
        // Commonly used instance of this template
        template <> class FeatureProvider<Feature>;
    }
}