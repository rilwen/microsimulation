/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_MUTABLE_CONTEXT_H
#define __AVERISERA_MS_MUTABLE_CONTEXT_H

#include "actor.hpp"
#include <memory>
#include <random>
#include <unordered_map>
#include <vector>
#include "core/dates_fwd.hpp"
#include "core/preconditions.hpp"
#include "core/rng.hpp"
#include "population.hpp"

namespace averisera {
    namespace microsim {
		class ImmutableContext;
        class Person;
        
        /** @brief Encapsulates shared data which change during the simulation.
         */
        class MutableContext {
        public:
			typedef size_t date_idx_t;
            /**
             \ pa*ram[in] seed Seed for the random number generator.
             */
            MutableContext(long seed = 42);

            /** Takes ownership of this implementation of RNG.
              @throw std::domain_error If rngimpl is null.
            */
            MutableContext(std::unique_ptr<RNG>&& rngimpl);

            MutableContext(const MutableContext&) = default; 
            
            /** Provide random number generator. */
            RNG& rng() {
                return *_rng;
            }
            
            /** Return current schedule period index. */
			date_idx_t date_index() const {
                return date_idx_;
            }
            
            /** Only call it from the Simulator. */
            void advance_date_index() {
                ++date_idx_;
            }
            
            /** Generate a new ID for an Actor object.
             * @throw std::runtime_error If run out of IDs
             */
            Actor::id_t gen_id();

			/** Return current maximum ID */
			Actor::id_t get_max_id() const {
				return _max_id;
			}

			/** Increase maximum ID to new value 
			@throw std::domain_error If new_max_id is less than current value.
			*/
			void increase_id(Actor::id_t new_max_id);

			/** Return newborns cache */
            const std::vector<std::shared_ptr<Person>>& newborns_cache() {
                return _newborns;
            }

			/** Remove all newborns from the cache */
			void wipe_out_newborns();

			/** Add new borns.
			@param babies Vector of Person pointers */
			void add_newborns(const std::vector<std::shared_ptr<Person>>& babies);

			/** Can be called multiple times for the same date 
			@param emigrants Must be sorted by IDs 
			*/
			void add_emigrants(const std::vector<std::shared_ptr<Person>>& emigrants, Date emigration_date);

			/** Return map of persons who left the simulated population due to emigration: date -> persons who emigrated on this date */
			const std::unordered_map<Date, std::vector<std::shared_ptr<Person>>>& emigrants() const {
				return emigrants_;
			}

			/** Add new immigrants */
			void add_immigrants(const std::vector<std::shared_ptr<Person>>& immigrants);

			/** Return vector of persons who joined the simulated population due to immigration */
			const std::vector<std::shared_ptr<Person>>& immigrants() const {
				return immigrants_;
			}

			/** Return current as of date */
			Date asof(const ImmutableContext& imm_ctx) const;			

			/** Return emigrant population object */
			const Population& emigrant_population() const {
				return emigrant_population_;
			}

			friend class Simulator;
        private:
            std::unique_ptr<RNG> _rng;
			date_idx_t date_idx_; /** Current schedule date index */
            Actor::id_t _max_id;
            std::vector<std::shared_ptr<Person>> _newborns; /** New born babies. Sorted by ID. */
			std::unordered_map<Date, std::vector<std::shared_ptr<Person>>> emigrants_; /**< Persons who left the simulated population due to emigration: map emigration date -> persons who emigrated on this date. Each value in map is sorted by ID. */
			Population emigrant_population_; /**< Another structure containing the emigrants for the purpose of simulating their mortality and procreation */
			std::vector<std::shared_ptr<Person>> immigrants_; /**< Persons who joined the simulated population due to immigration. Sorted by ID */

			Population& emigrant_population() {
				return emigrant_population_;
			}
        };
    }
}

#endif
