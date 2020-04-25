/*
(C) Averisera Ltd 2017
*/
#include "observer_demographics_emigrants.hpp"
#include "../contexts.hpp"
#include "../mutable_context.hpp"

namespace averisera {
	namespace microsim {
        void ObserverDemographicsEmigrants::observe(const Population&, const Contexts& ctx) {
            for (const auto& kv: ctx.mutable_ctx().emigrants()) {
                observe_persons(kv.second, ctx);
            }
        }
    }
}
