#include "../history_factory.hpp"
#include "operator_markov_model_actor.hpp"
#include "operator_pregnancy.hpp"
#include "../person.hpp"
#include "../person_data.hpp"
#include "../procreation.hpp"
#include "core/log.hpp"
#include <cassert>
#include <limits>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
		static auto HISTORY_FACTORY() {
            static auto hf = HistoryFactory::DENSE<uint8_t>();
            return hf;
        }
        
		static std::shared_ptr<const Predicate<Person>> LIVE_FEMALES_PRED() {
            return PredicateFactory::make_sex_shared(Sex::FEMALE, true);   
        }

        static std::unordered_set<Feature> get_required_features(const std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>>& relative_risks_transitions) {
            LOG_TRACE() << "get_required_features: get_required_features==" << relative_risks_transitions;            
            return FeatureUser<Feature>::combine(FeatureUser<Feature>::feature_set_t({ Procreation::CHILD_GENERATION() }),
                                                 FeatureUser<Feature>::combine(FeatureUser<Feature>::feature_set_t({ Procreation::CONCEPTION_FEATURE() }), FeatureUser<Feature>::gather_required_features(relative_risks_transitions, false)));
        }

        OperatorPregnancy::OperatorPregnancy(Pregnancy&& pregnancy,
                                             std::shared_ptr<const Predicate<Person>> predicate,
                                             std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>>&& relative_risks_transitions,
                                             unsigned int min_childbearing_age,
                                             unsigned int max_childbearing_age,
                                             bool dummy)
            : OperatorIndividual<Person>(false,
                                         FeatureUser<Feature>::feature_set_t({Procreation::PREGNANCY_FEATURE()}),
                                         get_required_features(relative_risks_transitions)),
                                                                           hist_gen_(Procreation::PREGNANCY_EVENT(), HISTORY_FACTORY(), LIVE_FEMALES_PRED()), // use 8-bit type for pregnancy events because there is much less than 256 of them
                                                                           history_user_(Procreation::CONCEPTION()),
                                                                           _pregnancy(std::move(pregnancy)), _pred(predicate), min_childbearing_age_(min_childbearing_age),
                                                                           max_childbearing_age_(min_childbearing_age)
		{
            check_not_null(predicate);
			check_greater_or_equal(max_childbearing_age, min_childbearing_age, "OperatorPregnancy: max childbearing age should be greater or equal min childbearing age");
			check_less(max_childbearing_age, std::numeric_limits<unsigned int>::max(), "OperatorPregnancy: max childbearing age should be less than maximum integer value");
            const auto nm = _pregnancy.nbr_stage_models();
            assert(nm > 0);
            if (relative_risks_transitions.size() != nm) {
                throw std::domain_error("OperatorPregnancy: bad relative risk vector size");
            }
            _operators.reserve(nm);
            for (Pregnancy::size_type i = 0; i < nm; ++i) {
                MarkovModel model(_pregnancy.stage_model(i));
                _operators.push_back(OperatorMarkovModelActor<Person>(Feature::empty(),
                                                                      Procreation::PREGNANCY_EVENT(),
                                                                      std::move(model),
                                                                      predicate,
                                                                      false,
                                                                      std::move(relative_risks_transitions[i]),
                                                                      std::vector<std::shared_ptr<const RelativeRisk<Person>>>(_pregnancy.stage_model(i).dim()),
                                                                      nullptr,
                                                                      HISTORY_FACTORY()
                                         ));
            }
            _cumulative_transition_counts.resize(nm);
			if (nm > 1) {
				_cumulative_transition_counts.front() = _pregnancy.transition_count(0);
				for (size_t i = 1; i < (nm - 1); ++i) {
					_cumulative_transition_counts[i] = _cumulative_transition_counts[i - 1] + _pregnancy.transition_count(static_cast<unsigned int>(i));
				}
			}
            _cumulative_transition_counts.back() = std::numeric_limits<unsigned int>::max();
        }

		OperatorPregnancy::OperatorPregnancy(Pregnancy&& pregnancy,
                                             std::shared_ptr<const Predicate<Person>> pred,
                                             std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>>&& relative_risks_transitions,
                                             unsigned int min_childbearing_age,
                                             unsigned int max_childbearing_age)
			: OperatorPregnancy(std::move(pregnancy),
                                wrap(pred, min_childbearing_age, max_childbearing_age),
                                std::move(relative_risks_transitions),
                                min_childbearing_age,
                                max_childbearing_age,
                                true) {}

		std::unique_ptr<OperatorPregnancy> OperatorPregnancy::build_with_empty_relative_risks(Pregnancy&& pregnancy,
                                                                                              std::shared_ptr<const Predicate<Person>> pred,
                                                                                              unsigned int min_childbearing_age,
                                                                                              unsigned int max_childbearing_age) {
            auto err = empty_relative_risks(pregnancy);
			return std::make_unique<OperatorPregnancy>(std::move(pregnancy), pred, std::move(err), min_childbearing_age, max_childbearing_age);
		}

        void OperatorPregnancy::apply(const std::shared_ptr<Person>& obj, const Contexts& contexts) const {
            assert(!_operators.empty());
			assert(obj->sex() == Sex::FEMALE);
            Date first_relevant_date = contexts.asof();
			if (contexts.mutable_ctx().date_index() == 0) {
				first_relevant_date = first_relevant_date - Period::months(Pregnancy::PREGNANCY_IN_MONTHS);
				first_relevant_date = std::max(first_relevant_date, obj->date_of_birth());
				LOG_TRACE() << "OperatorPregnancy: moving first relevant date from " << contexts.asof() << " to " << first_relevant_date;
			}

            auto date_event = get_last_date_and_event(*obj, contexts);
			first_relevant_date = std::max(first_relevant_date, date_event.first + Period::days(1)); // move start date to at least 1 day past the last pregnancy event and ignore all conception events before that date
            if (resulting_state(date_event.second) == Pregnancy::State::NOT_PREGNANT) { // last event ended a pregnancy
                const auto conc_hist_idx = obj->get_variable_index(contexts, Procreation::CONCEPTION());
                const ImmutableHistory& conc_history = obj->history(conc_hist_idx);
                // check if any conceptions in current period which could start a pregnancy
				// assume there is only 1 such event (the last one)
				if (!conc_history.empty()) {
                    Date last_conception_date = conc_history.last_date();
                    if (last_conception_date >= first_relevant_date) {
                        // initialize pregnancy history
                        if (last_conception_date > obj->date_of_birth()) {
							try {
								last_conception_date = set_next_event(*obj, last_conception_date, Pregnancy::Event::CONCEPTION, contexts);
								LOG_TRACE() << "OperatorPregnancy: initialised pregnancy on " << last_conception_date;
							} catch (std::domain_error& e) {
								LOG_ERROR() << "OperatorPregnancy: error \"" << e.what() << "\" while initialising pregnancy history from " << last_conception_date << " as of " << contexts.asof() << " on person: " << obj->to_data(contexts.immutable_ctx());
								throw e;
							}
							date_event = std::make_pair(last_conception_date, Pregnancy::Event::CONCEPTION);
                        } else {
							LOG_ERROR() << "OperatorPregnancy: conception date " << last_conception_date << " or on before date of birth " << obj->date_of_birth();
                            throw std::logic_error("OperatorPregnancy: conception on or before date of birth");
                        }
					} // else ignore this conception event as already handled
                }
            }

            const SchedulePeriod sp = contexts.current_period();
            ImmutableHistory::index_t trcnt = 0;
            unsigned int model_idx = 0;
            if (resulting_state(date_event.second) != Pregnancy::State::NOT_PREGNANT) {
				// count # of transitions it took to reach current stage of pregnancy
                trcnt = transitions_since_conception(*obj, contexts, date_event.first);
                const auto it = std::upper_bound(_cumulative_transition_counts.begin(), _cumulative_transition_counts.end(), trcnt);
                if (it == _cumulative_transition_counts.end()) {
                    // unlikely
                    model_idx = _pregnancy.nbr_stage_models() - 1;
                } else {
                    model_idx = static_cast<unsigned int>(std::distance(_cumulative_transition_counts.begin(), it));
                }
            }
            std::vector<double> rrvs(static_cast<unsigned int>(Pregnancy::Event::SIZE)); // TODO: avoid repeated allocation, maybe use std::array and pass a naked pointer to operators?
			
            while ((date_event.first < sp.end) && resulting_state(date_event.second) != Pregnancy::State::NOT_PREGNANT) {
                if (Pregnancy::is_terminating(date_event.second)) {
                    //date_state = std::make_pair(date_state.first  + Period::days(1), Pregnancy::State::NOT_PREGNANT);   
					break;
                } else {
                    const OperatorMarkovModelActor<Person>& op = _operators[model_idx];
					// new date can be past sp.end
                    const auto new_date_event_as_int = op.update_date_and_state(*obj, contexts, std::make_pair(date_event.first, static_cast<unsigned int>(date_event.second)), rrvs);
					assert(new_date_event_as_int.first >= date_event.first);
                    assert(new_date_event_as_int.second < static_cast<unsigned int>(Pregnancy::Event::SIZE));
					if (new_date_event_as_int.first == date_event.first) {
						LOG_ERROR() << "OperatorPregnancy: Markov model " << model_idx << " did not advance the date from " << date_event.first << " onwards when transitioning from non-terminating event " << date_event.second << " to " << static_cast<Pregnancy::Event>(new_date_event_as_int.second);
						throw std::logic_error("OperatorPregnancy: non-terminating event has zero transition period");
					}
                    date_event = std::make_pair(new_date_event_as_int.first, static_cast<Pregnancy::Event>(new_date_event_as_int.second));
                }
				date_event.first = set_next_event(*obj, date_event.first, date_event.second, contexts);
                ++trcnt;
                if (trcnt == _cumulative_transition_counts[model_idx] && trcnt < std::numeric_limits<unsigned int>::max()) {
                    ++model_idx;
                }
            }
        }

		std::pair<Date, Pregnancy::State> OperatorPregnancy::get_last_date_and_state(const Person& obj, const Contexts& ctx) const {
			const auto lde(get_last_date_and_event(obj, ctx));
			return std::make_pair(lde.first, resulting_state(lde.second));
		}

		std::pair<Date, Pregnancy::Event> OperatorPregnancy::get_last_date_and_event(const Person& obj, const Contexts& ctx) const {
			const auto& op = _operators.front();
			if (op.is_initialized(obj, ctx)) {
				const auto as_int = op.get_last_date_and_state(obj, ctx);
				assert(as_int.second < static_cast<unsigned int>(Pregnancy::Event::SIZE));
				return std::make_pair(as_int.first, static_cast<Pregnancy::Event>(as_int.second));
			} else {
				return std::make_pair(obj.date_of_birth(), Pregnancy::Event::SIZE);
			}
        }
        
        bool OperatorPregnancy::is_initialized(const Person& obj, const Contexts& contexts) const {
            return _operators.front().is_initialized(obj, contexts);
        }

        Date OperatorPregnancy::set_next_event(Person& obj, const Date date, const Pregnancy::Event evt, const Contexts& ctx) const {
			//if (is_initialized(obj, ctx)) {
			//	// check if the date is not too early
			//	const auto last = get_last_date_and_state(obj, ctx);
			//	if (date == last.first) {
			//		LOG_WARN() << "OperatorPregnancy: date " << date << " set twice on person " << obj.to_data(ctx.immutable_ctx()) << ", first with value " << last.second << " then with " << state << ". Moving date 1D forward";
			//		date = date + Period::days(1);
			//	}
			//}
			_operators.front().set_next_state(obj, date, static_cast<unsigned int>(evt), ctx);
			return date;
        }

        ImmutableHistory::index_t OperatorPregnancy::transitions_since_conception(const Person& obj, const Contexts& contexts, Date current_date) const {
            const auto hist_idx = obj.get_variable_index(contexts, Procreation::PREGNANCY_EVENT());
            const ImmutableHistory& history = obj.history(hist_idx);            
            const ImmutableHistory::index_t curr_idx = history.last_index(current_date);
            auto idx = curr_idx;
            while (idx > 0) {
                const auto next_idx = idx - 1;
                const ImmutableHistory::int_t next_st = history.as_int(next_idx);
                if (resulting_state(static_cast<Pregnancy::Event>(next_st)) == Pregnancy::State::NOT_PREGNANT) {
                    break;
                } else {
                    idx = next_idx;
                }
            }
            return curr_idx - idx;
        }

		std::shared_ptr<const Predicate<Person>> OperatorPregnancy::wrap(std::shared_ptr<const Predicate<Person>> external_predicate,
                                                                         unsigned int min_childbearing_age,
                                                                         unsigned int max_childbearing_age) {
            LOG_TRACE() << "OperatorPregnancy::wrap: external_predicate==" << external_predicate;
            LOG_TRACE() << "OperatorPregnancy::wrap: min_childbearing_age==" << min_childbearing_age;
            LOG_TRACE() << "OperatorPregnancy::wrap: max_childbearing_age==" << max_childbearing_age;
			std::shared_ptr<const Predicate<Person>> pred = LIVE_FEMALES_PRED();
			if (external_predicate) {
				pred = PredicateFactory::make_and({ external_predicate, pred });
			}
            LOG_TRACE() << "OperatorPregnancy::wrap: pred==" << max_childbearing_age;
			// do not apply the operator to too young women
			const unsigned int leave_years = static_cast<unsigned int>(std::ceil(static_cast<double>(Pregnancy::PREGNANCY_IN_MONTHS) / 12.0));
			if (min_childbearing_age > leave_years) {
				pred = pred->product(PredicateFactory::make_age(min_childbearing_age - leave_years, max_childbearing_age + 1, true));
			} else {
				pred = pred->product(PredicateFactory::make_max_age(max_childbearing_age + 1, true));
			}
			return pred;
		}

		std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>> OperatorPregnancy::empty_relative_risks(const Pregnancy& pregnancy) {
            //LOG_TRACE() << "OperatorPregnancy::empty_relative_risks: entering";
			const auto nm = pregnancy.nbr_stage_models();
            //LOG_TRACE() << "OperatorPregnancy::empty_relative_risks: nm==" << nm;
			std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>> risks(nm);
			for (Pregnancy::size_type i = 0; i < nm; ++i) {
				const auto dim = pregnancy.stage_model(i).dim();
                //LOG_TRACE() << "OperatorPregnancy::empty_relative_risks: dim[" << i << "]==" << dim;
				risks[i].ensure_region(0, 0, dim, dim);
			}
            //LOG_TRACE() << "OperatorPregnancy::empty_relative_risks: returning";
			return risks;
		}
    }
}
