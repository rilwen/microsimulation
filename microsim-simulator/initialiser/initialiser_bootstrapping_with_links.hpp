#pragma once
#include "../actor.hpp"
#include "initialiser_bootstrapping.hpp"
#include <cstdint>
#include <unordered_set>

namespace averisera {
    namespace microsim {
        /*! Initialiser selects randomly a Person from the sample and then adds its linked Persons (mother, children) recursively. */
        class InitialiserBootstrappingWithLinks: public InitialiserBootstrapping {
        public:
            typedef int depth_t;
            /*!
            \param recursion_limit Maximum distance in generations of the Person cloned from the sample from the one randomly drawn from it. If < 0 then we assume == 0.
            */
            InitialiserBootstrappingWithLinks(std::unique_ptr<PersonDataSampler>&& person_data_sampler, std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>&& person_perturbations, depth_t recursion_limit);

			InitialiserBootstrappingWithLinks(std::unique_ptr<PersonDataSampler>&& person_data_sampler, depth_t recursion_limit)
				: InitialiserBootstrappingWithLinks(std::move(person_data_sampler), std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>(), recursion_limit) {}

            InitialiserBootstrappingWithLinks& operator=(const InitialiserBootstrappingWithLinks&) = delete;
        private:
            void sample(std::vector<PersonData>& added_persons, pop_size_t remaining_size, const Contexts& ctx) const override;
			typedef std::unordered_set<Actor::id_t> visited_t;
			template <class F> pop_size_t walk_link_graph(const PersonData& person, const PersonData* from, depth_t depth, visited_t& visited, F action) const;

            const depth_t _recursion_limit;
        };
    }
}
