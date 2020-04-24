#include "operator_factory.hpp"
#include "operator/mortality.hpp"
#include "operator/mortality_enforcer.hpp"
#include "operator/operator_birth.hpp"
#include "operator/operator_conception.hpp"
#include "operator/operator_fetus_generator_simple.hpp"
#include "operator/operator_inheritance.hpp"
#include "operator/operator_pregnancy.hpp"
#include "operator/operator_markov_model_actor.hpp"

namespace averisera {
    namespace microsim {
        namespace OperatorFactory {
            std::unique_ptr<Operator<Person>> make_mortality(const HazardModel& hazard_model, const std::vector<std::shared_ptr<const RelativeRisk<Person>>>& relative_risks, std::shared_ptr<const Predicate<Person>> predicate, std::unique_ptr<Schedule>&& schedule, bool move_to_birth_date) {
                return std::unique_ptr<Operator<Person>>(new Mortality(hazard_model, relative_risks, predicate, std::move(schedule), move_to_birth_date));
            }

            std::unique_ptr<Operator<Person>> make_mortality_enforcer(std::shared_ptr<const Predicate<Person>> predicate, const std::vector<std::shared_ptr<const GenericDistribution<bool>>>& distributions) {
                return std::unique_ptr<Operator<Person>>(new MortalityEnforcer(predicate, distributions));
            }

            std::unique_ptr<Operator<Person>> make_inheritance(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<Person>> predicate,
                                                                     const Array2D<std::shared_ptr<const Distribution>>& mother_distributions,
                                                                     const Array2D<std::shared_ptr<const Distribution>>& child_distributions,
                                                                     const std::vector<std::shared_ptr<const CopulaGaussian>>& copulas,
                                                                     unsigned int date_offset, std::shared_ptr<const OperatorInheritance::ReferenceDateType> ref_date_type,
                                                                     const std::vector<HistoryFactory::factory_t>& history_factories) {
                return std::unique_ptr<Operator<Person>>(new OperatorInheritance(variables, predicate, mother_distributions, child_distributions, copulas, date_offset, ref_date_type, history_factories));
            }

			std::unique_ptr<Operator<Person>> make_pregnancy(Pregnancy&& pregnancy, std::shared_ptr<const Predicate<Person>> pred, unsigned int min_childbearing_age, unsigned int max_childbearing_age) {
				return OperatorPregnancy::build_with_empty_relative_risks(std::move(pregnancy), pred, min_childbearing_age, max_childbearing_age);
			}

			std::unique_ptr<Operator<Person>> make_birth(std::shared_ptr<const Predicate<Person>> predicate) {
				return std::make_unique<OperatorBirth>(predicate);
			}

			std::unique_ptr<Operator<Person>> make_birth(unsigned int min_childbearing_age, unsigned int max_childbearing_age) {
				return std::make_unique<OperatorBirth>(min_childbearing_age, max_childbearing_age);
			}

			std::unique_ptr<Operator<Person>> make_conception(const Conception& conception, const std::vector<std::shared_ptr<const RelativeRisk<Person>>>& relative_risks, std::shared_ptr<const Predicate<Person>> pred, std::unique_ptr<Schedule>&& schedule, const std::unique_ptr<const HazardRateMultiplierProvider<Person>>& hrm_provider, unsigned int min_childbearing_age, unsigned int max_childbearing_age, Period zero_fertility_period) {
				auto ptr = std::make_unique<OperatorConception>(conception, relative_risks, pred, std::move(schedule), min_childbearing_age, max_childbearing_age, zero_fertility_period);
				if (hrm_provider != nullptr) {
					ptr->add_hazard_rate_multiplier_provider(hrm_provider->clone());
				}
				return std::move(ptr);
			}

			std::unique_ptr<Operator<Person>> make_fetus_generator_simple(MarkovModel&& ethnicity_model, TimeSeries<Date, double>&& probs_female, unsigned int min_childbearing_age, unsigned int max_childbearing_age) {
				return std::make_unique<OperatorFetusGeneratorSimple>(std::move(ethnicity_model), std::move(probs_female), min_childbearing_age, max_childbearing_age);
			}

			std::unique_ptr<Operator<Person>> make_fetus_generator_simple(TimeSeries<Date, double>&& probs, unsigned int min_childbearing_age, unsigned int max_childbearing_age) {
				return std::make_unique<OperatorFetusGeneratorSimple>(std::move(probs), min_childbearing_age, max_childbearing_age);
			}

			std::unique_ptr<Operator<Person>> make_fetus_generator_simple(unsigned int min_childbearing_age, unsigned int max_childbearing_age, double prob_female) {
				return std::make_unique<OperatorFetusGeneratorSimple>(min_childbearing_age, max_childbearing_age, prob_female);
			}
        }        
    }
}
