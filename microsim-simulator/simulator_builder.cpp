#include "contexts.hpp"
#include "dispatcher_factory.hpp"
//#include "feature.h"
#include "history_factory_registry.hpp"
#include "immutable_context.hpp"
#include "operator.hpp"
#include "simulator_builder.hpp"
#include "simulator.hpp"
#include "core/preconditions.hpp"
#include <unordered_map>
#include <stdexcept>
#include <utility>
#include <vector>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
        SimulatorBuilder::SimulatorBuilder()
            : _initial_population_size(0), _add_newborns(true) {
        }
        
		SimulatorBuilder& SimulatorBuilder::add_operator(std::shared_ptr<Operator<Person>> op) {
            if (!op) {
                throw std::domain_error("SimulatorBuilder: null person operator");
            }
            _person_operators.push_back(op);
			return *this;
        }
        
		SimulatorBuilder& SimulatorBuilder::add_observer(std::shared_ptr<Observer> obs) {
            if (!obs) {
                throw std::domain_error("SimulatorBuilder: null observer");
            }
            _observers.push_back(obs);
			return *this;
        }

		SimulatorBuilder& SimulatorBuilder::set_add_newborns(bool new_value) {
            _add_newborns = new_value;
			return *this;
        }

		SimulatorBuilder& SimulatorBuilder::add_required_feature(const Simulator::feature_type& feature) {
			required_features_.insert(feature);
			return *this;
		}

		SimulatorBuilder& SimulatorBuilder::add_required_features(const Simulator::feature_set_type& features) {
			required_features_.insert(features.begin(), features.end());
			return *this;
		}

		SimulatorBuilder& SimulatorBuilder::set_initial_population_size(size_t new_value) {
			_initial_population_size = new_value;
			return *this;
		}

		SimulatorBuilder& SimulatorBuilder::add_migration_generator(const std::shared_ptr<const MigrationGenerator>& migration_generator) {
			check_not_null(migration_generator, "SimulatorBuilder: null migration generator");
			migration_generators_.push_back(migration_generator);
			return *this;
		}

		SimulatorBuilder& SimulatorBuilder::set_intermediate_observer_results_filename(const std::string& value) {
			intermediate_observer_results_filename_ = value;
			return *this;
		}
        
        Simulator SimulatorBuilder::build(Contexts&& ctx) {			
			collect_history_requirements(ctx.immutable_ctx());
			return Simulator(std::move(ctx), std::move(_person_operators), std::move(_observers), std::move(migration_generators_), _add_newborns, _initial_population_size, std::move(required_features_), std::move(intermediate_observer_results_filename_));
        }

        void SimulatorBuilder::collect_history_requirements(ImmutableContext& imm_ctx) {
			imm_ctx.collect_history_requirements(_person_operators);
        }

    }
}
