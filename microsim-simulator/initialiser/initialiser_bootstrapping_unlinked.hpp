// (C) Averisera Ltd 2014-2020
#pragma once
#include "initialiser_bootstrapping.hpp"

namespace averisera {
    namespace microsim {
        /** Samples Person objects but does not maintain links to children and mother. */
        class InitialiserBootstrappingUnlinked : public InitialiserBootstrapping {
        public:
            InitialiserBootstrappingUnlinked(std::unique_ptr<PersonDataSampler>&& person_data_sampler, std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>&& person_perturbations);

			InitialiserBootstrappingUnlinked(std::unique_ptr<PersonDataSampler>&& person_data_sampler)
				: InitialiserBootstrappingUnlinked(std::move(person_data_sampler), std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>()) {}
        private:
            void sample(std::vector<PersonData>& added_persons, pop_size_t remaining_size, const Contexts& ctx) const override;
        };
    }
}
