#include <memory>
#include "helpers.hpp"
#include "testing/rng_precalc.hpp"
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/immutable_context.hpp"
#include "microsim-simulator/mutable_context.hpp"

namespace averisera {
    namespace microsim {
        Contexts ctx_with_rng_precalc(const std::vector<double>& sample) {
            Contexts ctx(std::make_shared<ImmutableContext>(), std::make_shared<MutableContext>(std::unique_ptr<RNG>(new testing::RNGPrecalc(sample))));
            return ctx;
        }
    }
}
