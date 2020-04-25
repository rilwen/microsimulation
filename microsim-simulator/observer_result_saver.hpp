#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "core/dates_fwd.hpp"

namespace averisera {
    namespace microsim {
        class ImmutableContext;
        class Observer;

        /** Handles saving the results from Observer */
        class ObserverResultSaver {
        public:
            virtual ~ObserverResultSaver();

            /** Save intermediate results during the course of the simulation 
             @param asof Date at which the results are saved */
            virtual void save_intermediate(const Observer& observer, const ImmutableContext& imm_ctx, Date asof) = 0;

            /** Save final simulation results */
            virtual void save_final(const Observer& observer, const ImmutableContext& imm_ctx) = 0;
        };
    }
}
