// (C) Averisera Ltd 2014-2020
#pragma once
#include <vector>

namespace averisera {
    namespace microsim {
		class Contexts;

        /** Perturbs the bootstrapped data object
        @tparam AD ActorData or derived struct */
        template <class AD> class DataPerturbation {
        public:
            virtual ~DataPerturbation() {}

            /** Apply perturbation.
            */
            virtual void apply(std::vector<AD>& datas, const Contexts& ctx) const = 0;
        };
    }
}
