// (C) Averisera Ltd 2014-2020
#include "migration_generator_model.hpp"
#include "migrant_selector.hpp"
#include "../contexts.hpp"
#include "../person.hpp"
#include "../population.hpp"
#include "../population_data.hpp"
#include "../predicate.hpp"
#include "core/bootstrap.hpp"
#include "core/log.hpp"
#include "core/preconditions.hpp"
#include "core/rng.hpp"
#include <algorithm>
#include <cassert>
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		MigrationGeneratorModel::MigrationGeneratorModel(std::string&& name, std::vector<pred_model_pair>&& models, 
			const unsigned int comigrated_child_age_limit, std::shared_ptr<const MigrantSelector> migrant_selector)
		: comigrated_child_age_limit_(comigrated_child_age_limit), migrant_selector_(migrant_selector), name_(std::move(name))
		{
			check_not_null(migrant_selector);
			check_that(std::all_of(models.begin(), models.end(), [](const pred_model_pair& p) {
				return p.first != nullptr;
			}), "MigrationGeneratorModel: null predicate");
			if (models.empty()) {
				LOG_WARN() << "MigrationGeneratorModel: no models";
			}
			models_ = std::move(models);

			// enforce minimum age
			for (auto& pm : models_) {
				check_not_null(pm.first);
				auto new_pred = PredicateFactory::make_and(std::move(pm.first), PredicateFactory::make_min_age(comigrated_child_age_limit, true));
				pm = std::make_pair(std::move(new_pred), std::move(pm.second));
			}
		}

		void MigrationGeneratorModel::migrate_persons(const Population& population, const Contexts& ctx, std::vector<std::shared_ptr<Person>>& persons_removed, std::vector<PersonData>& persons_added) const {
			persons_removed.clear();
			persons_added.clear();
			std::vector<std::shared_ptr<Person>> selected;
			const SchedulePeriod sp = ctx.current_period();
			const Date migration_date = MigrationGenerator::calc_migration_date(sp);
			const Date asof = sp.begin;
			//const double dt = MigrationModel::calc_dt(sp.begin, sp.end);
			const std::vector<std::shared_ptr<Person>> live_persons(population.live_persons(asof));
			for (const pred_model_pair& mp : models_) {
				selected.clear();
				const Predicate<Person>& pred = *mp.first;
				if (!pred.active(ctx.asof())) {
					LOG_TRACE() << "MigrationGeneratorModel: pred " << pred << " inactive as of " << asof;
					continue;
				}
				if (pred.selects_alive_only()) {
					for (const std::shared_ptr<Person>& person_ptr : live_persons) {
						assert(person_ptr);
						if (pred.select_alive(*person_ptr, ctx)) {
							selected.push_back(person_ptr);
						}
					}
				} else {
					for (const std::shared_ptr<Person>& person_ptr : population.persons()) {
						assert(person_ptr);
						if (pred.select(*person_ptr, ctx)) {
							selected.push_back(person_ptr);
						}
					}
				}
				if (remove_below_age(ctx.asof(), comigrated_child_age_limit_, selected)) {
					LOG_WARN() << "MigrationGeneratorModel(" << name_ << "): found children below co-migration age in the sample selected by predicate " << pred.as_string();
				}
				const size_t size = selected.size();
				if (!size) {
					LOG_WARN() << "MigrationGeneratorModel: active model with predicate " << pred << " selected 0 persons as of " << ctx.asof();
					continue;
				}
				LOG_DEBUG() << "MigrationGeneratorModel: selected " << size << " Persons using predicate " << pred << " as of " << ctx.asof();
				const double x0 = static_cast<double>(size);				
				//const auto rate = mp.second.get_rate(sp.begin);
				//LOG_TRACE() << "MigrationGeneratorModel: using rate " << rate;
				const double dx = mp.second.calculate_migration(sp.begin, sp.end, x0); // MigrationModel::calculate_migration(x0, dt, rate);
				double x1 = MathUtils::random_round(x0 + dx, ctx.mutable_ctx().rng());
				if (x1 <= 0.0) {
					LOG_WARN() << "MigrationGeneratorModel: migration causes population segment selected by predicate " << pred << " to vanish as of " << asof << "; migration rate == " << mp.second.get_rate(sp.begin) << "; start population == " << x0 << "; interval == " << MigrationModel::calc_dt(sp.begin, sp.end) << "; new population == " << x1;
					x1 = 0;
				}				
				const size_t new_size = static_cast<size_t>(x1);
				LOG_DEBUG() << "MigrationGeneratorModel: size " << size << " changed to " << new_size << " due to migration";
				if (new_size < size) {
					// remove randomly selected individuals
					const size_t nbr_removed = size - new_size;
					const size_t old_removed_size = persons_removed.size();
					migrant_selector_->select(ctx, selected, persons_removed, nbr_removed);
					const size_t nbr_comigrated_children = comigrate_children(persons_removed, migration_date, comigrated_child_age_limit_, old_removed_size);
					LOG_TRACE() << "old_removed_size=" << old_removed_size << ", nbr_removed=" << nbr_removed << ", nbr_comigrated_children=" << nbr_comigrated_children << ", new_removed_size=" << persons_removed.size();
;				} else if (new_size > size) {
					// add more randomly selected individuals					
					const size_t nbr_added = new_size - size;
					const size_t old_added_size = persons_added.size();
					std::vector<std::shared_ptr<Person>> persons_added_ptrs(nbr_added);
					RNG::StlWrapper stl_rng(ctx.mutable_ctx().rng());
					Bootstrap<RNG::StlWrapper> bootstrap(stl_rng);
					bootstrap.resample_with_replacement(selected, persons_added_ptrs); // WARNING! this may lead to duplicate persons being selected
					// do children
					const size_t nbr_comigrated_children = comigrate_children(persons_added_ptrs, migration_date, comigrated_child_age_limit_, 0);
					convert_added_persons_to_data(ctx, persons_added_ptrs, comigrated_child_age_limit_, persons_added, migration_date);
					LOG_TRACE() << "old_added_size=" << old_added_size << ", nbr_added=" << nbr_added << ", nbr_comigrated_children=" << nbr_comigrated_children << ", new_removed_size=" << persons_removed.size();
				}
			}
			for (auto& pd : persons_added) {
				if (pd.date_of_birth <= migration_date) {
					pd.immigration_date = migration_date;
					if (!pd.date_of_death.is_not_a_date()) {
						pd.immigration_date = std::min(pd.immigration_date, pd.date_of_death);
					}
				}
			}
		}		
	}
}
