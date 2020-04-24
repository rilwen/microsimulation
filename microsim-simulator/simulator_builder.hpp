#ifndef __AVERISERA_MICROSIM_SIMULATOR_BUILDER_H
#define __AVERISERA_MICROSIM_SIMULATOR_BUILDER_H

#include "feature.hpp"
#include <memory>
#include <vector>
#include <unordered_set>

namespace averisera {
    namespace microsim {
        class Contexts;
		class ImmutableContext;
		class MigrationGenerator;
        class Observer;
        template <class T> class Operator;
        class Person;
        class Simulator;

        /*! Helps build Simulator objects */
        class SimulatorBuilder {
        public:
            SimulatorBuilder();
            
            /*! Add an Operator acting on Person objects.
              \throw std::domain_error If op is null */
			SimulatorBuilder& add_operator(std::shared_ptr<Operator<Person>> op);

			/*! Add an Operator acting on T objects (moves the operator)
			\throw std::domain_error If op is null */
			template <class T> SimulatorBuilder& add_operator(std::unique_ptr<Operator<T>>&& op) {
				return add_operator(std::shared_ptr<Operator<T>>(std::move(op)));
			}

			template <class T> SimulatorBuilder& add_operators(std::vector<std::unique_ptr<Operator<T>>>&& operators) {
				for (auto& op : operators) {
					add_operator(std::move(op));
				}
				return *this;
			}

            /*! \throw std::domain_error If obs is null */
			SimulatorBuilder& add_observer(std::shared_ptr<Observer> obs);

            /*! Set "add_newborns" property of Simulator (defaulted to true). */
			SimulatorBuilder& set_add_newborns(bool new_value);

			/*! Add required feature */
			SimulatorBuilder& add_required_feature(const Feature& feature);

			SimulatorBuilder& add_required_features(const std::unordered_set<Feature>& features);

			/*! Set initial population size. */
			SimulatorBuilder& set_initial_population_size(size_t new_value);

			/*! Set migration generator
			\throw std::domain_error If migration_generator is null */
			SimulatorBuilder& add_migration_generator(const std::shared_ptr<const MigrationGenerator>& migration_generator);

			/*! Set intermediate Observer results filename */
			SimulatorBuilder& set_intermediate_observer_results_filename(const std::string& value);
            
            /*! Builds a Simulator object and clears the state of the builder 
              \param ctx Contexts to use (moved)			  
             */
            Simulator build(Contexts&& ctx);
        private:
            void collect_history_requirements(ImmutableContext& imm_ctx);
           
            std::vector<std::shared_ptr<Operator<Person>>> _person_operators;
            std::vector<std::shared_ptr<Observer>> _observers;
			std::vector<std::shared_ptr<const MigrationGenerator>> migration_generators_;
            size_t _initial_population_size;			
			bool _add_newborns;
			std::unordered_set<Feature> required_features_;
			std::string intermediate_observer_results_filename_;
        };
    }
}

#endif // __AVERISERA_MICROSIM_SIMULATOR_BUILDER_H
