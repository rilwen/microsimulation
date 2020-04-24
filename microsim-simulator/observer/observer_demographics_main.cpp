/*
(C) Averisera Ltd 2017
*/
#include "observer_demographics_main.hpp"
#include "../population.hpp"

namespace averisera {
	namespace microsim {
        void ObserverDemographicsMain::observe(const Population& population, const Contexts& ctx) {
            observe_persons(population.persons(), ctx);
        }
    }
}
