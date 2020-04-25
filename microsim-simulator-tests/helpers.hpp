#pragma once
#include <vector>

namespace averisera {
    namespace microsim {
        class Contexts;
        
        /** Contexts with RNGPrecalc */
        Contexts ctx_with_rng_precalc(const std::vector<double>& sample);

    }
}
