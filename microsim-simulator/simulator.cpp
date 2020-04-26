// (C) Averisera Ltd 2014-2020
#include "feature.hpp"
#include "feature_provider.hpp"
#include "initialiser.hpp"
#include "immutable_context.hpp"
#include "migration_generator.hpp"
#include "mutable_context.hpp"
#include "observer.hpp"
#include "operator.hpp"
#include "person.hpp"
#include "population.hpp"
#include "population_data.hpp"
#include "predicate.hpp"
#include "predicate/pred_alive.hpp"
#include "simulator.hpp"
#include "microsim-core/schedule.hpp"
#include "core/log.hpp"
#include "core/preconditions.hpp"
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <boost/functional/hash.hpp>

namespace averisera {
    namespace microsim {
		const Simulator::feature_set_type IGNORED_REQUIREMENTS({ INSTANTANEOUS() });

        Simulator::Simulator(Contexts&& ctx,
                             std::vector<std::shared_ptr<Operator<Person>>>&& person_operators,
                             std::vector<std::shared_ptr<Observer>>&& observers,
			std::vector<std::shared_ptr<const MigrationGenerator>>&& migration_generators,
                             bool add_newborns,
			size_t initial_population_size,
			feature_set_type&& required_features,
			std::string&& intermediate_observer_results_filename
            )
            : person_operator_performance_(person_operators.size()),
			_init_pop_size(initial_population_size), _add_newborns(add_newborns)
        {
            validate(person_operators, observers, migration_generators, required_features);
            _ctx = std::move(ctx);
            _person_operators = std::move(person_operators);
			FeatureProvider<Feature>::sort(_person_operators);
            _observers = std::move(observers);
			migration_generators_ = std::move(migration_generators);
			required_features_ = std::move(required_features);
			intermediate_observer_results_filename_ = std::move(intermediate_observer_results_filename);
            person_operators.resize(0);
            observers.resize(0);            
			LOG_INFO() << "Simulator: " << _person_operators.size() << " Person operators";
			size_t idx = 0;
			for (const std::shared_ptr<Operator<Person>>& op : _person_operators) {
				LOG_INFO() << "Simulator::person_operator[" << idx << "]: " << op->name() << ", PRED=" << op->predicate().as_string() << ", PROV=" << op->provides() << ", REQS=" << op->requires();
				++idx;
			}
			LOG_INFO() << "Simulator: " << _observers.size() << " observers";
			LOG_INFO() << "Simulator: initial population size: " << _init_pop_size;
			LOG_INFO() << "Simulator: add newborns: " << _add_newborns;
			LOG_INFO() << "Simulator: ethnicity conversions: " << _ctx.immutable_ctx().ethnicity_conversions();
        }

		Simulator::Simulator(Simulator&& other)
			: _ctx(std::move(other._ctx)),
			_person_operators(std::move(other._person_operators)),
			_observers(std::move(other._observers)),
			migration_generators_(std::move(other.migration_generators_)),
			_init_pop_size(other._init_pop_size),
			_add_newborns(other._add_newborns),
			required_features_(std::move(other.required_features_)),
				intermediate_observer_results_filename_(std::move(other.intermediate_observer_results_filename_))				
		{
			FeatureProvider<Feature>::sort(_person_operators);
			other._person_operators.resize(0);
			other._observers.resize(0);
		}

		Simulator& Simulator::operator=(Simulator&& other) {
			if (this != &other) {
				_ctx = std::move(other._ctx);
				_person_operators = std::move(other._person_operators);
				_observers = std::move(other._observers);
				migration_generators_ = std::move(other.migration_generators_);
				_add_newborns = other._add_newborns;
				_init_pop_size = other._init_pop_size;
				required_features_ = std::move(other.required_features_);
				intermediate_observer_results_filename_ = std::move(other.intermediate_observer_results_filename_);
				other._person_operators.resize(0);
				other._observers.resize(0);
				other.intermediate_observer_results_filename_.clear();
			}
			return *this;
		}
	
        void Simulator::apply_operator(Population& population, const std::vector<std::shared_ptr<Person>>& live_persons, const Operator<Person>& op, const size_t op_idx, const bool is_main) const {
            const Predicate<Person>& predicate = op.predicate();
			const Date asof = _ctx.asof();
			check_that(predicate.active(asof), "Simulator::apply_operator: operator is not active");
			std::vector<std::shared_ptr<Person>> selected;
			selected.reserve(population.persons().size());
			if (predicate.selects_alive_only()) {
				for (const auto& person_ptr : live_persons) {
					assert(person_ptr);
					assert(person_ptr->date_of_birth() <= asof);
					if (predicate.select_alive(*person_ptr, _ctx)) {
						selected.push_back(person_ptr);
					}
				}
			} else {
				for (const auto& person_ptr : population.persons()) {
					assert(person_ptr);
					if (person_ptr->date_of_birth() > asof) {
						if (is_main) {
							LOG_ERROR() << "Simulator: person's date of birth " << person_ptr->date_of_birth() << " is after asof " << asof << " in population " << population.name() << " when applying operator " << op.name() << " with predicate " << predicate;
							throw std::logic_error("Simulator: Person born in the future");
						} else {
							continue;
						}
					}
					if (predicate.select(*person_ptr, _ctx)) {
						selected.push_back(person_ptr);
					}
				}
			}
			
			LOG_DEBUG() << "Simulator: operator " << op.name() << " (" << op_idx << ") was active and selected " << selected.size() << " persons as of " << _ctx.asof() << " using predicate " << op.predicate().as_string() << " on " << (is_main ? "MAIN" : "AUXILIARY") << " population";

			if (!selected.empty()) {
				// measure operator performance
				Performance& perf = person_operator_performance_[op_idx];
				perf.measure_metrics([&op, &selected, this]() {
					op.apply(selected, _ctx);
				}, selected.size());
			}
        }

        void Simulator::apply_operators(Population& population, const bool is_main) const {
            std::vector<std::shared_ptr<Operator<Person> > > active_operators;
			std::vector<size_t> active_operator_indices;
			active_operators.reserve(_person_operators.size());
			active_operator_indices.reserve(_person_operators.size());
			size_t op_idx = 0;
            for (const std::shared_ptr<Operator<Person> >& op: _person_operators) {
				check_that(op != nullptr, "Simulator::apply_operator: null operator");
                if (op->is_active(_ctx.asof())) {
                    active_operators.push_back(op);
					active_operator_indices.push_back(op_idx);
                }
				++op_idx;
            }
			LOG_INFO() << "Simulator: " << active_operators.size() << " active Person operators as of " << _ctx.asof();
			FeatureProvider<Feature>::sort(active_operators);
			check_active_operators(population, active_operators);
			const Date asof = _ctx.asof(); 
			const std::vector<std::shared_ptr<Person>> live_persons(population.live_persons(asof));
			size_t active_op_idx = 0;
			for (const std::shared_ptr<Operator<Person> >& op : active_operators) {
				check_that(op != nullptr, "Simulator::apply_operator: null operator");
				apply_operator(population, live_persons, *op, active_operator_indices[active_op_idx], is_main);
				++active_op_idx;
			}
        }

		void Simulator::check_active_operators(const Population& population, const std::vector<std::shared_ptr<Operator<Person>>>& active_operators) const {
			if (!FeatureProvider<Feature>::are_all_requirements_satisfied(active_operators, IGNORED_REQUIREMENTS, required_features_)) {
				throw std::runtime_error("Simulator: not all requirements for Person Operaxtors satisfied by the active set");
			}
			typedef std::vector<std::shared_ptr<Operator<Person>>> operator_vec_t;
			const size_t nops = active_operators.size();
			// select which operator to apply to which person without involving the context
			std::vector<bool> opsel(nops);
			std::unordered_set<operator_vec_t, boost::hash<operator_vec_t>> selections;
			for (auto pit = population.persons().begin(); pit != population.persons().end(); ++pit) {
				const auto person = *pit;
				assert(person);
				size_t nsel = 0;
				auto opsel_it = opsel.begin();
				for (const std::shared_ptr<Operator<Person> >& op : active_operators) {
					assert(op);
					const bool s = op->predicate().select_out_of_context(*person);
					*opsel_it = s;
					if (s) {
						++nsel;
					}
					++opsel_it;
				}
				assert(opsel_it == opsel.end());
				operator_vec_t operators;
				operators.reserve(nsel);
				opsel_it = opsel.begin();
				for (auto aop_it = active_operators.begin(); aop_it != active_operators.end(); ++aop_it, ++opsel_it) {
					if (*opsel_it) {
						operators.push_back(*aop_it);
					}
				}
				assert(opsel_it == opsel.end());
				selections.insert(operators);
			}

			// check if operators are consistent
			for (auto mit = selections.begin(); mit != selections.end(); ++mit) {
				const operator_vec_t& operators = *mit;
				if (!FeatureProvider<Feature>::are_all_requirements_satisfied(operators, IGNORED_REQUIREMENTS, required_features_)) {
					throw std::runtime_error("Simulator: not all requirements for Person Operators satisfied");
				}
			}
		}

        void Simulator::apply_observers(Population& population) const {
            std::for_each(_observers.begin(), _observers.end(), [this, &population](const std::shared_ptr<Observer>& obs) {
                    obs->observe(population, _ctx);
                });
        }

        void Simulator::validate(const std::vector<std::shared_ptr<Operator<Person>>>& person_operators,
                                 const std::vector<std::shared_ptr<Observer>>& observers,
			const std::vector<std::shared_ptr<const MigrationGenerator>>& migration_generators,
			const feature_set_type& required_features) const {
			if (!FeatureProvider<Feature>::are_all_requirements_satisfied(person_operators, IGNORED_REQUIREMENTS, required_features)) {
                // catches some but not all problems
                throw std::runtime_error("Simulator::validate: all not requirements for Person Operators satisfied");
            }
            if (std::any_of(observers.begin(), observers.end(), [](const std::shared_ptr<Observer>& obs) { return !obs; })) {
                throw std::domain_error("Simulator: null observer");
            }
			check_that(_init_pop_size != 0, "Simulator: initial population size cannot be zero");
			check_that(!migration_generators.empty(), "Simulator: no migration generators");
			check_all_not_null(migration_generators, "Simulator: some migration generators are null");
        }

        void Simulator::transfer_initialised_members(Population& initialised_pool, Population& simulated_pop) const {
            static const PredAlive pred_alive;
            simulated_pop.transfer_persons(initialised_pool, pred_alive, _ctx);
        }

        void Simulator::add_newborns(Population& population) const {
			LOG_DEBUG() << "Simulator: adding " << _ctx.mutable_ctx().newborns_cache().size() << " newborns on " << _ctx.asof();
            population.add_persons(_ctx.mutable_ctx().newborns_cache());
			_ctx.mutable_ctx().wipe_out_newborns();
        }

        static void unlink_children(Population& population, Date since) {
            for (const Person::shared_ptr& person: population.persons()) {
                person->unlink_children(since);                
            }
        }

        void Simulator::step(Population& population, const bool is_main) const {
			const auto sp = _ctx.current_period();
			const clock_t time0 = std::clock();
            apply_operators(population, is_main);
			if (_add_newborns) {
                add_newborns(population);
            } else {
                unlink_children(population, _ctx.asof());
            }
			if (is_main) {
				apply_observers(population);
				if (sp.end > sp.begin) {
					apply_migration(population);					
				}
			}			
			const clock_t time1 = std::clock();
			LOG_INFO() << "Simulator: step for population " << population.name() << " with size " << population.persons().size() << " from " << sp.begin << " to " << sp.end << " took " << (static_cast<double>(time1 - time0) * 1000.0) / CLOCKS_PER_SEC << " miliseconds";
        }

		const Schedule& Simulator::simulation_schedule() const {
			return _ctx.immutable_ctx().schedule();
		}

		void Simulator::initialise_population(const Initialiser& initialiser, Population& population) const {
			const clock_t time0 = std::clock();
			if (!population.empty()) {
				throw std::domain_error("Simulator: population must be empty");
			}
			PopulationData population_data(initialiser.initialise(_init_pop_size, _ctx));
			population.import_data(population_data, _ctx, true, false); // initialiser should ensure correct IDs
			const clock_t time1 = std::clock();
			LOG_INFO() << "Simulator: population initialisation took " << (static_cast<double>(time1 - time0) * 1000.0) / CLOCKS_PER_SEC << " miliseconds";
		}

		void Simulator::run(Population& population) const {
			const clock_t time0 = std::clock();
			while (_ctx.asof_idx() < simulation_schedule().nbr_dates()) {
				// update the emigrant population (mortality, births) - do this first so that we don't handle the same person twice
				step(_ctx.mutable_ctx().emigrant_population(), false);
				// update the main population
				step(population, true);
				// save intermediate results if not yet finished
				if (_ctx.asof_idx() < simulation_schedule().nbr_dates()) {
					for (const auto& obs_ptr: _observers) {
                        obs_ptr->save_intermediate_results(_ctx.immutable_ctx(), _ctx.asof());
                    }
				}
				_ctx.mutable_ctx().advance_date_index();
			}
			const clock_t time1 = std::clock();
			LOG_INFO() << "Simulator: simulation for population " << population.name() << " took " << static_cast<double>(time1 - time0) / CLOCKS_PER_SEC << " seconds";
			log_operator_performance();
		}

        void Simulator::save_observer_results() const {
            for (const auto& obs_ptr: _observers) {
                obs_ptr->save_final_results(_ctx.immutable_ctx());
            }
        }

		void Simulator::log_operator_performance() const {
			std::stringstream perf_ss;
			perf_ss << "Operator\tPredicate\tNbrElements\tTotalTime\tMeanTimePerStep\tMeanTimePerElement\tMeanTimePerElementPerStep\n";
			for (size_t i = 0; i < _person_operators.size(); ++i) {
				const auto name = _person_operators[i]->name();
				const auto pred_str = _person_operators[i]->predicate().as_string();
				const auto& perf = person_operator_performance_[i];
				perf_ss << name;
				perf_ss << "\t" << pred_str;
				perf_ss << "\t" << perf.total_nbr_processed();
				perf_ss << "\t" << perf.total_time();
				perf_ss << "\t" << perf.total_time_stats().mean();
				perf_ss << "\t" << perf.total_time() / static_cast<double>(perf.total_nbr_processed());
				perf_ss << "\t" << perf.time_per_element_stats().mean();
				perf_ss << "\n";
			}
			LOG_INFO() << "Simulator: Person operator performance statistics:\n" << perf_ss.str();
		}

		void Simulator::apply_migration(Population& population) const {
			const auto sp = _ctx.current_period();
			const clock_t time0 = std::clock();
			const auto migration_date = MigrationGenerator::calc_migration_date(sp);
			//size_t idx = 0;
			for (const auto& mg : migration_generators_) {
				std::vector<std::shared_ptr<Person>> removed_persons;
				PopulationData added_population;
				mg->migrate_persons(population, _ctx, removed_persons, added_population.persons);
				LOG_INFO() << "Simulator: migration generator " << mg->name() << " on " << _ctx.asof() << ": adding " << added_population.persons.size() << ", removing " << removed_persons.size();
				Population::sort_persons(removed_persons);				
				const size_t nbr_removed_persons = removed_persons.size();
				removed_persons.erase(std::unique(removed_persons.begin(), removed_persons.end(), Person::equal_ptr_by_id), removed_persons.end());
				if (removed_persons.size() < nbr_removed_persons) {
					LOG_WARN() << "Simulator::apply_migration: migration generator " << mg->name() << " returned duplicate persons for removal as of " << sp.begin;
				}
				population.remove_persons(removed_persons);
				_ctx.mutable_ctx().add_emigrants(removed_persons, migration_date);
				population.import_data(added_population, _ctx, true, true);
				//++idx;
			}			
			const clock_t time1 = std::clock();
			LOG_INFO() << "Simulator: migration over " << sp.begin << " to " << sp.end << " took " << (static_cast<double>(time1 - time0) * 1000.0) / CLOCKS_PER_SEC << " miliseconds";
		}
    }
}
