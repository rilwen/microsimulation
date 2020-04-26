// (C) Averisera Ltd 2014-2020
#include "../contexts.hpp"
#include "../mutable_context.hpp"
#include "operator_birth.hpp"
#include "../person.hpp"
#include "../predicate_factory.hpp"
#include "../procreation.hpp"
#include "microsim-core/pregnancy.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {

        OperatorBirth::OperatorBirth(std::shared_ptr<const Predicate<Person>> pred)
            : OperatorIndividual<Person>(false, FeatureUser<Feature>::feature_set_t({Procreation::BIRTH_FEATURE()}), FeatureUser<Feature>::feature_set_t({Procreation::PREGNANCY_FEATURE()}))
            , history_user_(Procreation::PREGNANCY_EVENT())
            ,  _pred(pred) {
            if (!_pred) {
				_pred = PredicateFactory::make_sex(Sex::FEMALE, true);
            }
        }

		static std::shared_ptr<const Predicate<Person>> make_sex_age_predicate(unsigned int min_childbearing_age, unsigned int max_childbearing_age) {
			check_greater_or_equal(max_childbearing_age, min_childbearing_age, "OperatorBirth::OperatorBirth");
			auto pred_sex = PredicateFactory::make_sex_shared(Sex::FEMALE, true);
			return pred_sex->product(PredicateFactory::make_age(min_childbearing_age, max_childbearing_age, true));
		}

		OperatorBirth::OperatorBirth(unsigned int min_childbearing_age, unsigned int max_childbearing_age)
			: OperatorBirth(make_sex_age_predicate(min_childbearing_age, max_childbearing_age)) {}
        
        void OperatorBirth::apply(const std::shared_ptr<Person>& obj, const Contexts& contexts) const {
            const auto hist_idx = obj->get_variable_index(contexts, Procreation::PREGNANCY_EVENT());
            const ImmutableHistory& history = obj->history(hist_idx);
            const SchedulePeriod sp = contexts.current_period();
            unsigned int prev_nbr_children = obj->nbr_children();
			const Date asof = contexts.asof();
            if (!history.empty() && history.last_date() >= asof) {
                History::index_t idx = history.first_index(asof);
				unsigned int nbr_terminating_events = 0;
				Date event_date;
                while (idx < history.size() && (event_date = history.date(idx)) < sp.end) {
                    const Pregnancy::Event evt = static_cast<Pregnancy::Event>(history.as_int(idx));
                    if (Pregnancy::is_terminating(evt)) {
						++nbr_terminating_events;
                        switch (evt) {
                        case Pregnancy::Event::MISCARRIAGE:
                            obj->remove_fetuses(event_date);
                            break;
                        case Pregnancy::Event::BIRTH:
                            obj->give_birth(event_date, contexts, obj);
                            break;
                        default:
                            throw std::logic_error("OperatorBirth: unknown pregnancy event");
                        }
						if (nbr_terminating_events > 1) {
							LOG_WARN() << "OperatorBirth: 2nd or later terminating pregnancy event " << evt << " at date " << event_date;
						}
                        const unsigned int new_nbr_children = obj->nbr_children();
                        if (new_nbr_children > prev_nbr_children) {
							LOG_TRACE() << "OperatorBirth: caching " << (new_nbr_children - prev_nbr_children) << " newborns on " << event_date;
							std::vector<std::shared_ptr<Person>> babies;
							babies.reserve(new_nbr_children - prev_nbr_children);                            
                            for (unsigned int cidx = prev_nbr_children; cidx < new_nbr_children; ++cidx) {
                                babies.push_back(obj->get_child(cidx));
                            }
							contexts.mutable_ctx().add_newborns(babies);
                        }
						prev_nbr_children = new_nbr_children;
                    }
					++idx;
                }
				if (nbr_terminating_events > 1) {
					LOG_WARN() << "OperatorBirth: more than 1 terminating pregnancy event in period " << sp;
				}
            }
        }
    }
}
