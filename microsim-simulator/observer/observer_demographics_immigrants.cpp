/*
(C) Averisera Ltd 2017
*/
#include "observer_demographics_immigrants.hpp"
#include "../contexts.hpp"
#include "../mutable_context.hpp"

namespace averisera {
	namespace microsim {
        void ObserverDemographicsImmigrants::observe(const Population& population, const Contexts& ctx) {
            observe_persons(ctx.mutable_ctx().immigrants(), ctx);
        }
    }
}
