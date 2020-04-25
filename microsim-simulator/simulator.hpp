#ifndef __AVERISERA_MS_SIMULATOR_H
#define __AVERISERA_MS_SIMULATOR_H

#include "contexts.hpp"
#include "feature.hpp"
#include "feature_user.hpp"
#include "performance.hpp"
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace averisera {
    namespace microsim {
		class Initialiser;
		class MigrationGenerator;
        class Observer;
        template <class T> class Operator;
        class Population;
        class Person;
		class Schedule;
	
        /** @brief Performs the simulation.

          Arguably the most important class in the library. 
         */
        class Simulator {
        public:
			typedef Feature feature_type;
			typedef FeatureUser<feature_type>::feature_set_t feature_set_type;

            /** Constructor which moves the arguments 
			@param person_operators Vector of operators, sorted internally according to what features are provided or required
			@param observers Vector of observers
			@param add_newborns Whether to include newly born children in the population (true) or just record birth dates (false)
			@param initial_population_size Size of initial population
			@param required_features Required features which have to be always provided by operators whether any operators requires them or not
			@param intermediate_observer_results_filename If not empty, after each simulation step save the Observer results to file with this name
			@throw std::runtime_error If relations between operators are inconsistent or feature requirements are not satisfied
			@throw std::domain_error If any pointer is null or initial_population_size is zero.
			*/
            Simulator(Contexts&& ctx, std::vector<std::shared_ptr<Operator<Person>>>&& person_operators,
				std::vector<std::shared_ptr<Observer>>&& observers, 
				std::vector<std::shared_ptr<const MigrationGenerator>>&& migration_generators,
				bool add_newborns, size_t initial_population_size, feature_set_type&& required_features,
				std::string&& intermediate_observer_results_filename
                );

            /** Move constructor */
            Simulator(Simulator&& other);

			/** Move assignment */
			Simulator& operator=(Simulator&& other);
            
            Simulator(const Simulator&) = delete;
            Simulator& operator=(const Simulator&) = delete;

			/** Run the simulation. Save intermediate Observer results if and how specified by their ObserverResultSaver members.
			*/
			void run(Population& population) const;

            /** Save final Observer results */
            void save_observer_results() const;

            /**  Transfer all currently alive members of initialised_pool to simulated_pop.
             */
            void transfer_initialised_members(Population& initialised_pool, Population& simulated_pop) const;

			/** Return simulation schedule */
			const Schedule& simulation_schedule() const;

			/** Given an empty population, initialise it. 
			@throw std::domain_error If population is not empty. 
			*/
			void initialise_population(const Initialiser& initialiser, Population& population) const;

			size_t initial_population_size() const {
				return _init_pop_size;
			}

			bool is_add_newborns() const {
				return _add_newborns;
			}
        private:
			/** Perform a simulation step, applying operators, handling births and doing all the necessary observations.
			Step is forward-looking, i.e. the step applied at T_i causes changes in the [T_i, T_{i+1}) period.
			@param is_main Is this the main population
			*/
			void step(Population& population, bool is_main) const;

			void validate(const std::vector<std::shared_ptr<Operator<Person>>>& person_operators,
                          const std::vector<std::shared_ptr<Observer>>& observers,
				const std::vector<std::shared_ptr<const MigrationGenerator>>& migration_generators,
				const feature_set_type& required_features) const;

            /** Apply operator to the population 
			@param op_idx Index of the operator in the _person_operators vector
			@param is_main Is this the main population
			*/
            void apply_operator(Population& population, const std::vector<std::shared_ptr<Person>>& live_persons, const Operator<Person>& op, size_t op_idx, bool is_main) const;

            /** Apply stored operators to the population
			@param is_main Is this the main population
            @throw std::runtime_error If relations between operators are inconsistent or feature requirements are not satisfied
            */
            void apply_operators(Population& population, bool is_main) const;

            /** Apply observers to gather information about simulation results */
            void apply_observers(Population& population) const;

			/** Assume active_operators are sorted w/r to feature requirements */
			void check_active_operators(const Population& population, const std::vector<std::shared_ptr<Operator<Person>>>& active_operators) const;

            /** Add all recently born children to the population */
            void add_newborns(Population& population) const;

			/** Subtract / add population members due to migration */
			void apply_migration(Population& population) const;

			void log_operator_performance() const;

            Contexts _ctx;
            std::vector<std::shared_ptr<Operator<Person>>> _person_operators;
			mutable std::vector<Performance> person_operator_performance_; // mutable because we update the statistics during execution
            std::vector<std::shared_ptr<Observer>> _observers;			
			std::vector<std::shared_ptr<const MigrationGenerator>> migration_generators_;
            size_t _init_pop_size;
			bool _add_newborns;
			feature_set_type required_features_;
			std::string intermediate_observer_results_filename_;
        };
    }
}

#endif //  __AVERISERA_MS_SIMULATOR_H
