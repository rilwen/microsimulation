#include "../contexts.hpp"
#include "initialiser_bootstrapping_unlinked.hpp"
#include "../mutable_context.hpp"
#include "../person.hpp"
#include "core/rng.hpp"
#include <cassert>

namespace averisera {
    namespace microsim {
        InitialiserBootstrappingUnlinked::InitialiserBootstrappingUnlinked(std::unique_ptr<PersonDataSampler>&& person_data_sampler, std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>&& person_perturbations)
            : InitialiserBootstrapping(std::move(person_data_sampler), std::move(person_perturbations)) {}

        void InitialiserBootstrappingUnlinked::sample(std::vector<PersonData>& added_persons, pop_size_t remaining_size, const Contexts& ctx) const {
            assert(remaining_size > 0);
            assert(added_persons.empty());
            const size_t idx = static_cast<size_t>(ctx.mutable_ctx().rng().next_uniform(sample_size() - 1));
            assert(idx < sample_size());
            const PersonData& sampled = sample_person(idx);
            add_copy(added_persons, sampled, ctx);
        }
    }
}
