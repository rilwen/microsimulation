// (C) Averisera Ltd 2014-2020
#include "../contexts.hpp"
#include "initialiser_bootstrapping.hpp"
#include "../mutable_context.hpp"
#include "../population_data.hpp"
#include "core/rng.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace averisera {
    namespace microsim {        
        InitialiserBootstrapping::InitialiserBootstrapping(std::unique_ptr<PersonDataSampler>&& person_data_sampler)
            : InitialiserBootstrapping(std::move(person_data_sampler), std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>()) {}

        InitialiserBootstrapping::InitialiserBootstrapping(std::unique_ptr<PersonDataSampler>&& person_data_sampler, std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>&& person_perturbations)
             {
            if (std::any_of(person_perturbations.begin(), person_perturbations.end(), [](const std::unique_ptr<const DataPerturbation<PersonData>>& ptr) { return !ptr; })) {
                throw std::domain_error("InitialiserBootstrapping: null perturbation");
            }            
			person_data_sampler_ = std::move(person_data_sampler);
			_person_perturbations = std::move(person_perturbations);            			
        }

        PopulationData InitialiserBootstrapping::initialise(const pop_size_t total_size, const Contexts& ctx) const {
            PopulationData population;
            std::vector<PersonData> added_persons;
            added_persons.reserve(1);
            pop_size_t remaining_size = total_size;
            while (remaining_size > 0) {
                added_persons.clear();
                sample(added_persons, remaining_size, ctx);
                assert(remaining_size >= added_persons.size());
                assert(added_persons.size() > 0);
                for (PersonData& p : added_persons) {
                    population.persons.push_back(std::move(p));
                }
                remaining_size -= added_persons.size();
            }
            for (const std::unique_ptr<const DataPerturbation<PersonData>>& pptr : _person_perturbations) {
				pptr->apply(population.persons, ctx);
            }
            return population;
        }

        void InitialiserBootstrapping::add_copy(std::vector<PersonData>& added_persons, const PersonData& person, const Contexts& ctx) {
            PersonData copy(person.clone_without_links(ctx.mutable_ctx().gen_id()));
            added_persons.push_back(std::move(copy));
        }

		InitialiserBootstrapping::PersonDataSamplerFromData::PersonDataSamplerFromData(std::vector<PersonData>&& sample) {
			if (sample.empty()) {
				throw std::domain_error("InitialiserBootstrapping::PersonDataSamplerFromData: no sample persons");
			}
			sample_persons_ = std::move(sample);
			ActorData::sort_by_id(sample_persons_);
		}

		size_t InitialiserBootstrapping::PersonDataSamplerFromData::find_by_id(Actor::id_t id) const {
			const auto it = ActorData::find_by_id(sample_persons_, id);
			if (it != sample_persons_.end()) {
				return std::distance(sample_persons_.begin(), it);
			} else {
				throw std::out_of_range("InitialiserBootstrapping::PersonDataSamplerFromData: could not find sample member with this ID");
			}
		}

		InitialiserBootstrapping::PersonDataSamplerFromPersons::PersonDataSamplerFromPersons(const std::vector<std::shared_ptr<Person>>& sample_persons, const ImmutableContext& imm_ctx)
			: sample_persons_(sample_persons), imm_ctx_(imm_ctx) {
			if (sample_persons.empty()) {
				throw std::domain_error("InitialiserBootstrapping::PersonDataSamplerFromPersons: no sample persons");
			}
		}

		const PersonData& InitialiserBootstrapping::PersonDataSamplerFromPersons::sample_person(size_t idx) const {
			assert(idx < sample_persons_.size());
			const auto it = _converted_cache.find(idx);
			if (it != _converted_cache.end()) {
				return it->second;
			} else {
				const auto tmp = _converted_cache.insert(std::make_pair(idx, sample_persons_[idx]->to_data(imm_ctx_)));
				assert(tmp.second);
				//LOG_TRACE() << "InitialiserBootstrapping::PersonDataSamplerFromPersons: cache size " << _converted_cache.size();
				return tmp.first->second;
			}
		}

		size_t InitialiserBootstrapping::PersonDataSamplerFromPersons::find_by_id(Actor::id_t id) const {
			const auto it = Actor::find_by_id<Person>(sample_persons_, id);
			if (it != sample_persons_.end()) {
				return std::distance(sample_persons_.begin(), it);
			} else {
				throw std::out_of_range("InitialiserBootstrapping::PersonDataSamplerFromPersons: could not find sample member with this ID");
			}
		}
    }
}
