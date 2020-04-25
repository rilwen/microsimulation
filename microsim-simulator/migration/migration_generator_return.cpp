#include "migration_generator_return.hpp"
#include "migrant_selector_random.hpp"
#include "../contexts.hpp"
#include "../person.hpp"
#include "../person_data.hpp"
#include "microsim-core/migration_model.hpp"
#include "core/preconditions.hpp"

namespace averisera {
	namespace microsim {
		MigrationGeneratorReturn::MigrationGeneratorReturn(std::string&& name, Date from, Date to, double fraction, unsigned int returning_children_age_limit, std::unique_ptr<EmigrantSelector>&& emigrant_selector)
			: from_(from), to_(to), fraction_(fraction), returning_children_age_limit_(returning_children_age_limit),
			name_(std::move(name))
		{
			check_that(fraction >= 0 && fraction <= 1, "MigrationGeneratorReturn: fraction must be in [0,1] range");
			check_that(from < to, "MigrationGeneratorReturn: return dates out of order");
			check_not_null(emigrant_selector, "MigrationGeneratorReturn: emigrant selector is null");
			check_equals(returning_children_age_limit, emigrant_selector->min_age(), "MigrationGeneratorReturn: inconsistent minimum age");
			emigrant_selector_ = std::move(emigrant_selector);
		}

		void MigrationGeneratorReturn::migrate_persons(const Population&, const Contexts& ctx, std::vector<std::shared_ptr<Person>>& persons_removed, std::vector<PersonData>& persons_added) const {
			persons_removed.clear();
			persons_added.clear();
			const SchedulePeriod sp = ctx.current_period();
			if (sp.begin >= from_ && sp.end <= to_) {
				std::vector<std::shared_ptr<Person>> available;
				emigrant_selector_->select(ctx, ctx.asof(), available);
				if (remove_below_age(ctx.asof(), returning_children_age_limit_, available)) {
					LOG_WARN() << "MigrationGeneratorReturn(" << name_ << "): found children below co-migration age in the sample selected by emigrant selector " << emigrant_selector_->predicate_as_string();
				}
				const double total_dt = MigrationModel::calc_dt(from_, to_);
				const double period_dt = MigrationModel::calc_dt(sp.begin, sp.end);
				const size_t nbr_returning = static_cast<size_t>(static_cast<double>(available.size()) * fraction_ * period_dt / total_dt);
				std::vector<std::shared_ptr<Person>> returning;
				MigrantSelectorRandom ms;
				ms.select(ctx, available, returning, nbr_returning);
				const Date immigration_date = MigrationGenerator::calc_migration_date(sp);
				comigrate_children(returning, immigration_date, returning_children_age_limit_, 0);
				convert_added_persons_to_data(ctx, returning, returning_children_age_limit_, persons_added, immigration_date);
			} else {
				check_that(sp.end <= to_ || sp.begin >= from_, "MigrationGeneratorReturn: Schedules are incompatible");
			}
		}

		
	}
}
