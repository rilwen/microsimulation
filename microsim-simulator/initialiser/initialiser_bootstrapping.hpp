// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_INITIALISER_BOOTSTRAPPING_HPP
#define __AVERISERA_MICROSIM_INITIALISER_BOOTSTRAPPING_HPP

#include "data_perturbation.hpp"
#include "../initialiser.hpp"
#include "../person.hpp"
#include "../person_data.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace averisera {
    namespace microsim {
        /** Initialiser which bootstraps a population from a set of Actor objects. */
        class InitialiserBootstrapping : public Initialiser {
        public:  
			/** Used to sample persons */
			class PersonDataSampler {
			public:
				virtual ~PersonDataSampler() {}

				/** Return sample size */
				virtual size_t sample_size() const = 0;

				/** Return idx-th person from the sample
				@param idx idx < sample_size() */
				virtual const PersonData& sample_person(size_t idx) const = 0;

				/** Find index of person with given ID or throw std::out_of_range if no such ID in sample */
				virtual size_t find_by_id(Actor::id_t id) const = 0;
			};

            /**
            @param sample_persons Sample of PersonData objects to draw from. Cannot be empty.
            @throw std::domain_error
            */
            InitialiserBootstrapping(std::unique_ptr<PersonDataSampler>&& person_data_sampler);

            /** @param sample_persons Sample of PersonData objects to draw from. Cannot be empty.
            @param person_perturbations Vector of perturbations to apply to each bootstrapped PersonData object. Elements cannot be null.
            */
            InitialiserBootstrapping(std::unique_ptr<PersonDataSampler>&& person_data_sampler, std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>&& person_perturbations);

            PopulationData initialise(pop_size_t total_size, const Contexts& ctx) const override;

			class PersonDataSamplerFromData: public PersonDataSampler {
			public:
				PersonDataSamplerFromData(std::vector<PersonData>&& sample);
					
				size_t sample_size() const override {
					return sample_persons_.size();
				}

				const PersonData& sample_person(size_t idx) const override {
					assert(idx < sample_persons_.size());
					return sample_persons_[idx];
				}

				size_t find_by_id(Actor::id_t id) const override;
			private:
				std::vector<PersonData> sample_persons_;
			};

			/** Stores references to Person sample and ImmutableContext. Converts Person to PersonData on demand. */
			class PersonDataSamplerFromPersons : public PersonDataSampler {
			public:
				/** @param sample_persons Non-empty vector of pointers to Person objects, sorted by ID 
				@throw std::domain_error If sample_persons is empty */
				PersonDataSamplerFromPersons(const std::vector<std::shared_ptr<Person>>& sample_persons, const ImmutableContext& imm_ctx);

				size_t sample_size() const override {
					return sample_persons_.size();
				}

				const PersonData& sample_person(size_t idx) const override;

				size_t find_by_id(Actor::id_t id) const override;
			private:
				const std::vector<std::shared_ptr<Person>>& sample_persons_;
				const ImmutableContext& imm_ctx_;
				mutable std::unordered_map<size_t, PersonData> _converted_cache;
			};
        protected:
			size_t sample_size() const {
				return person_data_sampler_->sample_size();
			}

			const PersonData& sample_person(size_t idx) const {
				return person_data_sampler_->sample_person(idx);
			}

			/** Find PersonData with given ID or throw std::out_of_range if no such ID in sample */
			size_t find_by_id(Actor::id_t id) const {
				return person_data_sampler_->find_by_id(id);
			}

            /** Add copy of person at the back of added_persons. Break all mother/child links. */
            static void add_copy(std::vector<PersonData>& added_persons, const PersonData& person, const Contexts& ctx);
        private:
			std::unique_ptr<PersonDataSampler> person_data_sampler_;            
            std::vector<std::unique_ptr<const DataPerturbation<PersonData>>> _person_perturbations;

            // Clear added_persons and sample at most remaining_size persons into it.
            virtual void sample(std::vector<PersonData>& added_persons, pop_size_t remaining_size, const Contexts& ctx) const = 0;			
        };
    }
}

#endif // __AVERISERA_MICROSIM_INITIALISER_BOOTSTRAPPING_HPP
