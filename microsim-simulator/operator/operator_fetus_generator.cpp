// (C) Averisera Ltd 2014-2020
#include "operator_fetus_generator.hpp"
#include "operator_conception.hpp"
#include "../contexts.hpp"
#include "../fetus.hpp"
#include "../feature.hpp"
#include "../mutable_context.hpp"
#include "../person.hpp"
#include "../predicate_factory.hpp"
#include "../procreation.hpp"
#include "microsim-core/sex.hpp"
#include "core/math_utils.hpp"

namespace averisera {
    namespace microsim {
        OperatorFetusGenerator::OperatorFetusGenerator(std::shared_ptr<const Predicate<Person>> pred, unsigned int min_childbearing_age, unsigned int max_childbearing_age)
            : OperatorIndividual<Person>(false,
                                         FeatureUser<Feature>::feature_set_t({Procreation::CHILD_GENERATION()}),
                                         FeatureUser<Feature>::feature_set_t({Procreation::CONCEPTION_FEATURE()})),
            history_user_(Procreation::CONCEPTION()),
			min_childbearing_age_(min_childbearing_age),
			max_childbearing_age_(max_childbearing_age)
		{
			check_greater_or_equal(max_childbearing_age, min_childbearing_age, "OperatorFetusGenerator: max childbearing age should be greater or equal min childbearing age");
            std::shared_ptr<const Predicate<Person>> preselection = PredicateFactory::make_and<Person>({PredicateFactory::make_sex(Sex::FEMALE)}); 
			// do not apply the operator to too young women
			const unsigned int leave_years = static_cast<unsigned int>(std::ceil(static_cast<double>(Pregnancy::PREGNANCY_IN_MONTHS) / 12.0));
			if (min_childbearing_age > leave_years) {
				preselection = preselection->product(PredicateFactory::make_age(min_childbearing_age - leave_years, max_childbearing_age, true));
			} else {
				preselection = preselection->product(PredicateFactory::make_max_age(max_childbearing_age, true));
			}
			if (!pred) {
                _pred = preselection;
            } else {
                _pred = preselection->product(pred);
            }
        }
            
        void OperatorFetusGenerator::apply(const std::shared_ptr<Person>& mother, const Contexts& contexts) const {
            const SchedulePeriod sp = contexts.current_period();
			Date start_date = sp.begin;
			if (contexts.mutable_ctx().date_index() == 0) { // are we at first simulation date?
				// Generate fetuses in the past to ensure that we don't have zero births for the first 9 months of simulation
				start_date = start_date - Period::months(Pregnancy::PREGNANCY_IN_MONTHS);
				LOG_TRACE() << "OperatorFetusGenerator: moving start date from " << sp.begin << " to " << start_date;
			}
			start_date = std::max(start_date, OperatorConception::calc_first_conception_date_allowed(mother, contexts, min_childbearing_age_, Period()));
			if (start_date >= sp.end) {
				return;
			}
			const auto hist_idx = mother->get_variable_index(contexts, Procreation::CONCEPTION());
            const ImmutableHistory& history = mother->history(hist_idx);
			const auto hist_size = history.size();
			if (!hist_size || history.last_date() < start_date) {
				return;
			}
            // find all conception events since start_date until but not including next simulation date.
			const auto event_idx_start = history.first_index(start_date);            
            bool got_pregnant = false;
			Date first_conception_date; // for debugging
            for (auto eid = event_idx_start; eid < hist_size; ++eid) {
                const Date conception_date = history.date(eid);
                if (conception_date < sp.end) {
                    if (got_pregnant) {
						LOG_WARN() << "OperatorFetusGenerator: more than one pregnancy in the period from " << start_date << " to " << sp.end << ", first at " << first_conception_date << " then at " << conception_date << ". Ignoring the second one";
						break;
                    } else {
                        got_pregnant = true;
						first_conception_date = conception_date;
                    }
                    const Conception::multiplicity_type multi = MathUtils::safe_cast<Conception::multiplicity_type>(history.as_int(eid));
                    const auto fetuses = generate_fetuses(mother, conception_date, multi, contexts);
                    for (const Fetus& fetus: fetuses) {
						try {
							mother->add_fetus(fetus);
						} catch (std::logic_error& e) {
							const auto pregn_hist_idx = mother->get_variable_index(contexts, Procreation::PREGNANCY_EVENT());
							const ImmutableHistory& pregn_hist = mother->history(pregn_hist_idx);
							const auto conc_hist_idx = mother->get_variable_index(contexts, Procreation::CONCEPTION());
							const ImmutableHistory& conc_hist = mother->history(conc_hist_idx);
							LOG_ERROR() << "OperatorFetusGenerator: problem with adding fetus conceived on " << conception_date << " in period from " << start_date << " to " << sp.end << " with pregnancy history " << pregn_hist.as_string() << " and conception history " << conc_hist.as_string() << " as of " << contexts.asof();
							throw e;
						}
                    }
                }
            }
        }
    }
}
