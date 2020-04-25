#ifndef __AVERISERA_MS_OBSERVER_H
#define __AVERISERA_MS_OBSERVER_H
#include "core/dates_fwd.hpp"
#include <iosfwd>
#include <memory>

namespace averisera {
    namespace microsim {
        class Contexts;
		class ImmutableContext;
        class ObserverResultSaver;
        class Population;
		class Schedule;
        
        /** Observes the population during the simulation and gathers results */
        class Observer {
        public:
            /** @param result_saver If null, don't try to call it to save results. */
            Observer(std::shared_ptr<ObserverResultSaver> result_saver);
            
            virtual ~Observer();

            /** Observe the population. Called once per simulation date. */
            virtual void observe(const Population& population, const Contexts& ctx) = 0;

            /** Save results to stream. Called by ObserverResultSaver.
			@param sim_schedule Simulation schedule
			*/
            virtual void save_results(std::ostream& os, const ImmutableContext& im_ctx) const = 0;

            /** Save intermediate results using the private result_saver 
             @param asof Date at which results are saved */
            void save_intermediate_results(const ImmutableContext& im_ctx, Date asof) const;

            /** Save final results using the private result_saver */
            void save_final_results(const ImmutableContext& im_ctx) const;
        private:
            std::shared_ptr<ObserverResultSaver> result_saver_;
        };
    }
}

#endif // __AVERISERA_MS_OBSERVER_H
