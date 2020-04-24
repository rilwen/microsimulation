#include "operator_conception.hpp"
#include "../person.hpp"
#include "../person_data.hpp"
#include "../predicate_factory.hpp"
#include "../procreation.hpp"
#include "../relative_risk.hpp"
#include "../relative_risk_factory.hpp"
#include "microsim-core/hazard_model.hpp"
#include "core/generic_distribution.hpp"
#include "core/log.hpp"
#include "core/math_utils.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        OperatorConception::OperatorConception(const Conception& conception, const std::vector<std::shared_ptr<const RelativeRisk<Person>>>& relative_risks, std::shared_ptr<const Predicate<Person>> external_predicate, std::unique_ptr<Schedule>&& schedule, unsigned int min_childbearing_age, unsigned int max_childbearing_age,
			Period zero_fertility_period)
            : OperatorIndividual<Person>(false, FeatureUser<Feature>::feature_set_t({Procreation::CONCEPTION_FEATURE()}), FeatureUser<Feature>::gather_required_features(relative_risks, true)),
			hist_use_(Procreation::PREGNANCY_EVENT()),
            hist_gen_(Procreation::CONCEPTION(), HistoryFactory::DENSE<Conception::multiplicity_type>(), predicate_for_history_generator()), // dense history because conception is an event
            _conception(conception), 
			_relative_risks(relative_risks), 
			_pred(predicate_for_operator(external_predicate, min_childbearing_age, max_childbearing_age)),
			_schedule(std::move(schedule)),
			min_childbearing_age_(min_childbearing_age),
			zero_fertility_period_(zero_fertility_period)
		{
			check_greater_or_equal(max_childbearing_age, min_childbearing_age, "OperatorConception: max childbearing age should be greater or equal min childbearing age");
			check_greater_or_equal(zero_fertility_period.size, 0);
        }

        void OperatorConception::apply(const std::shared_ptr<Person>& obj, const Contexts& ctx) const {
            assert(!_schedule || ctx.immutable_ctx().schedule().contains(*_schedule));
            const HazardModel hazard_model(_conception.hazard_model(obj->date_of_birth()));
            const SchedulePeriod sp = Operator<Person>::current_period(_schedule, ctx);
            // we begin by assuming that the Person is not pregnant
            bool conceived = false;
            Date date = sp.begin;
			assert(date <= sp.end);
			const Date first_conc_date_allowed(calc_first_conception_date_allowed(obj, ctx));
			if (ctx.mutable_ctx().date_index() == 0) { // are we at first simulation date?
				// Generate pregnancies in the past to ensure that we don't have zero births for the first 9 months of simulation
				date = date - Period::months(Pregnancy::PREGNANCY_IN_MONTHS);
				LOG_TRACE() << "OperatorConception(" << _pred->as_string() << "): moving start date from " << sp.begin << " to " << date;
			}
			date = std::max(date, first_conc_date_allowed);
            std::vector<HazardRateMultiplier> hazard_rate_multipliers(_relative_risks.size() + hrm_providers_.size());
            while ((date < sp.end) && (!conceived)) {
                auto hrm_it = hazard_rate_multipliers.begin();
                for (const auto& relative_risk: _relative_risks) {
                    assert(hrm_it != hazard_rate_multipliers.end());
                    const RelativeRiskValue relative_risk_value((*(relative_risk))(*obj, ctx));
                    *hrm_it = hazard_model.calc_hazard_rate_multiplier(0, relative_risk_value);
                    ++hrm_it;
                }
				for (const auto& hrmprov : hrm_providers_) {
					assert(hrm_it != hazard_rate_multipliers.end());
					*hrm_it = (*hrmprov)(*obj, ctx);
					++hrm_it;
				}
                const double jump_proba = hazard_model.calc_transition_probability(0, date, sp.end, hazard_rate_multipliers);
                const double u = ctx.mutable_ctx().rng().next_uniform();
                const Date next_date = hazard_model.calc_end_date(0, date, u, hazard_rate_multipliers);				
				assert(next_date >= date);
				date = next_date;
                if ((u < jump_proba) && (date < sp.end)) {
                    conceived = true;
                    break; // Only 1 pregnancy per simulation period suported
                }                
            }
            if (conceived) {
				const double age = obj->age_fract(date);
                const auto& multiplicity_distro = _conception.multiplicity(date.year(), age);
                const Conception::multiplicity_type multiplicity = multiplicity_distro.random(ctx.mutable_ctx().rng());
				LOG_TRACE() << "OperatorConception: person of age " << age << " and ethnicity " << ctx.immutable_ctx().ethnicity_conversions().name(obj->ethnicity()) << " conceived with multiplicity " << int(multiplicity) << " on " << date << ", sim date " << ctx.asof();
				const auto hist_idx = obj->get_variable_index(ctx, Procreation::CONCEPTION());
				History& history = obj->history(hist_idx);
				const auto value = MathUtils::safe_cast<History::int_t>(multiplicity);
				if (history.empty() || (history.last_date() < date)) {
					history.append(date, value);
				} else {
					LOG_WARN() << "OperatorConception: conception date on or after " << date << " already found as of " << ctx.asof() << " in person ID=" << obj->id() << ", AGE=" << age << ", ETHN=" << ctx.immutable_ctx().ethnicity_conversions().name(obj->attributes().ethnicity()) << ", DOB=" << obj->date_of_birth() << " while adding value " << value << " (last value " << history.last_as_int() << "); operator predicate is " << this->predicate().as_string();
				}
            }
        }

		std::shared_ptr<const Predicate<Person>> OperatorConception::predicate_for_history_generator() {
			return PredicateFactory::make_sex_shared(Sex::FEMALE, true);
		}

        std::shared_ptr<const Predicate<Person>> OperatorConception::predicate_for_operator(const std::shared_ptr<const Predicate<Person>>& external_predicate, unsigned int min_childbearing_age, unsigned int max_childbearing_age) {
			// check the state of pregnancy history at end of current period, so that we don't ignore any time period when conception
			// would be possible
			std::shared_ptr<const Predicate<Person>> result = PredicateFactory::make_and<Person>({ predicate_for_history_generator(), PredicateFactory::make_pregnancy(Pregnancy::State::NOT_PREGNANT, true, false) });
			// do not apply the operator to too young women
			const unsigned int leave_years = static_cast<unsigned int>(std::ceil(static_cast<double>(Pregnancy::PREGNANCY_IN_MONTHS) / 12.0));
			if (min_childbearing_age > leave_years) {
				result = result->product(PredicateFactory::make_age(min_childbearing_age - leave_years, max_childbearing_age, true));
			} else {
				result = result->product(PredicateFactory::make_max_age(max_childbearing_age, true));
			}
			// multiply by external predicate
            if (!external_predicate) {
                return result;
            } else {
                return result->product(external_predicate);
            }
        }

		Date OperatorConception::calc_first_conception_date_allowed(const std::shared_ptr<Person>& obj, const Contexts& ctx, unsigned const int min_childbearing_age, const Period zero_fertility_period) {
			check_greater_or_equal(zero_fertility_period.size, 0);
			Date date = (obj->date_of_birth() + Period::years(min_childbearing_age)) - Period::months(Pregnancy::PREGNANCY_IN_MONTHS);
			if (zero_fertility_period.size != 0) {
				const auto hist_idx = obj->get_variable_index(ctx, Procreation::PREGNANCY_EVENT());
				const ImmutableHistory& history = obj->history(hist_idx);
				if (!history.empty()) {
					const Pregnancy::Event evt = static_cast<Pregnancy::Event>(history.last_as_int());
					const Date last_date = history.last_date();
					if (Pregnancy::is_terminating(evt)) {
						// birth or miscarriage					
						date = std::max(date, last_date + zero_fertility_period);
					} else {
						LOG_WARN() << "OperatorConception: operator applied to Person " << obj->to_data(ctx.immutable_ctx()) << " with last pregnancy event " << evt << " at " << last_date;
						date = Date::MAX; // Person is pregnant now, no conception possible
					}
				}
			}
			return date;
		}

		Date OperatorConception::calc_first_conception_date_allowed(const std::shared_ptr<Person>& obj, const Contexts& ctx) const {
			return calc_first_conception_date_allowed(obj, ctx, min_childbearing_age_, zero_fertility_period_);
		}
    }
}
