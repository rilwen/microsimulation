#ifndef __AVERISERA_MS_OPERATOR_FACTORY_H
#define __AVERISERA_MS_OPERATOR_FACTORY_H

#include "hazard_rate_multiplier_provider.hpp"
#include "operator_group.hpp"
#include "operator/operator_hazard_model.hpp"
#include "operator/operator_hazard_model_actor.hpp"
#include "operator/operator_enforcer.hpp"
#include "operator/operator_incrementer.hpp"
#include "operator/operator_enforcer_multi.hpp"
#include "operator/operator_incrementer_multi.hpp"
#include "operator/operator_inheritance.hpp"
#include <memory>
#include <vector>

namespace averisera {
    template <class T> class Array2D;
    class DiscreteDistribution;
    namespace microsim {
		class Conception;
		template <class A> class HazardRateMultiplierProvider;
        class MarkovModel;
        class Person;
        class Pregnancy;
		template <class A> class RelativeRisk;
		class Schedule;

        /*! Factory methods for Operator implementations */
        namespace OperatorFactory {
            /*! \see OperatorGroup */
            template <class T> std::unique_ptr<Operator<T>> make_group(const std::vector<std::shared_ptr<const Operator<T>>>& operators, std::shared_ptr<Dispatcher<T, unsigned int>> dispatcher) {
                return std::unique_ptr<Operator<T>>(new OperatorGroup<T>(operators, dispatcher));
            }
            
            /*! \see OperatorHazardModel */
            template <class T> std::unique_ptr<Operator<T>> make_hazard_model(const std::string& state_variable, Feature provided_feature, const HazardModel& hazard_model, const std::vector<std::shared_ptr<const RelativeRisk<T>>>& relative_risks, std::shared_ptr<const Predicate<T>> predicate) {
                return std::unique_ptr<Operator<T>>(new OperatorHazardModel<T>(state_variable, provided_feature, hazard_model, relative_risks, predicate));
            }

            /*! \see OperatorHazardModelActor */
            template <class T> std::unique_ptr<Operator<T>> make_hazard_model_actor(const std::string& state_variable, Feature provided_feature, const HazardModel& hazard_model, const std::vector<std::shared_ptr<const RelativeRisk<T>>>& relative_risks, std::shared_ptr<const Predicate<T>> predicate, HistoryFactory::factory_t history_factory) {
                return std::unique_ptr<Operator<T>>(new OperatorHazardModelActor<T>(provided_feature, hazard_model, relative_risks, predicate, state_variable, history_factory));
            }
            
            /*! \see OperatorEnforcer */
            template <class T> std::unique_ptr<Operator<T>> make_enforcer(const std::string& variable, std::shared_ptr<const Predicate<T>> predicate,
                                                                                const std::vector<std::shared_ptr<const Distribution>>& distributions,
                                                                                HistoryFactory::factory_t history_factory, std::unique_ptr<Schedule>&& schedule) {
                return std::unique_ptr<Operator<T>>(new OperatorEnforcer<T>(variable, predicate, distributions, history_factory, std::move(schedule)));
            }

            /*! \see OperatorIncrementer */
            template <class T> std::unique_ptr<Operator<T>> make_incrementer(const std::string& variable, std::shared_ptr<const Predicate<T>> predicate, const std::vector<std::shared_ptr<const Distribution>>& distributions, std::unique_ptr<Schedule>&& schedule) {
                return std::unique_ptr<Operator<T>>(new OperatorIncrementer<T>(variable, predicate, distributions, std::move(schedule)));
            }

            /*! \see OperatorEnforcerMulti */
            template <class T> std::unique_ptr<Operator<T>> make_enforcer_multivariate(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<T>> predicate, const std::vector<std::shared_ptr<const MultivariateDistribution>>& distributions, const std::vector<HistoryFactory::factory_t>& history_factories) {
                return std::unique_ptr<Operator<T>>(new OperatorEnforcerMulti<T>(variables, predicate, distributions, history_factories));
            }

            /*! \see OperatorIncrementerMulti */
            template <class T> std::unique_ptr<Operator<T>> make_incrementer_multivariate(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<T>> predicate, const std::vector<std::shared_ptr<const MultivariateDistribution>>& distributions) {
                return std::unique_ptr<Operator<T>>(new OperatorIncrementerMulti<T>(variables, predicate, distributions));
            }

            /*! \see Mortality */
            std::unique_ptr<Operator<Person>> make_mortality(const HazardModel& hazard_model, const std::vector<std::shared_ptr<const RelativeRisk<Person>>>& relative_risks, std::shared_ptr<const Predicate<Person>> predicate, std::unique_ptr<Schedule>&& schedule, bool move_to_birth_date);

            /*! \see MortalityEnforcer */
            std::unique_ptr<Operator<Person>> make_mortality_enforcer(std::shared_ptr<const Predicate<Person>> predicate, const std::vector<std::shared_ptr<const DiscreteDistribution>>& distributions);

            /*! \see OperatorInheritance */
            std::unique_ptr<Operator<Person>> make_inheritance(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<Person>> predicate,
                                                                     const Array2D<std::shared_ptr<const Distribution>>& mother_distributions,
                                                                     const Array2D<std::shared_ptr<const Distribution>>& child_distributions,
                                                                     const std::vector<std::shared_ptr<const CopulaGaussian>>& copulas,
                                                                     unsigned int date_offset, std::shared_ptr<const OperatorInheritance::ReferenceDateType> ref_date_type,
                                                                     const std::vector<HistoryFactory::factory_t>& history_factories);

            /*! \see OperatorBirth */
            std::unique_ptr<Operator<Person>> make_birth(std::shared_ptr<const Predicate<Person>> predicate = nullptr);

			/*! \see OperatorBirth */
			std::unique_ptr<Operator<Person>> make_birth(unsigned int min_childbearing_age, unsigned int max_childbearing_age);

            /*! \see OperatorPregnancy */
            std::unique_ptr<Operator<Person>> make_pregnancy(Pregnancy&& pregnancy, std::shared_ptr<const Predicate<Person>> pred,
                                                                   std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>>& relative_risks_transitions);

			/*! \see OperatorPregnancy */
			std::unique_ptr<Operator<Person>> make_pregnancy(Pregnancy&& pregnancy, std::shared_ptr<const Predicate<Person>> pred, unsigned int min_childbearing_age, unsigned int max_childbearing_age);

            /*! \see OperatorFetusGeneratorSimple */
            std::unique_ptr<Operator<Person>> make_fetus_generator_simple(MarkovModel&& ethnicity_model, TimeSeries<Date, double>&& probs_female, unsigned int min_childbearing_age, unsigned int max_childbearing_age);

			/*! \see OperatorFetusGeneratorSimple */
			std::unique_ptr<Operator<Person>> make_fetus_generator_simple(TimeSeries<Date, double>&& probs_female, unsigned int min_childbearing_age, unsigned int max_childbearing_age);

			/*! \see OperatorFetusGeneratorSimple */
			std::unique_ptr<Operator<Person>> make_fetus_generator_simple(unsigned int min_childbearing_age, unsigned int max_childbearing_age, double prob_female = 0.5);

			/*! \see OperatorConception 
			\param hrm_provider If not null, it is added to the operator */
			std::unique_ptr<Operator<Person>> make_conception(const Conception& conception, const std::vector<std::shared_ptr<const RelativeRisk<Person>>>& relative_risks, std::shared_ptr<const Predicate<Person>> pred, std::unique_ptr<Schedule>&& schedule, const std::unique_ptr<const HazardRateMultiplierProvider<Person>>& hrm_provider, unsigned int min_childbearing_age, unsigned int max_childbearing_age, Period zero_fertility_period);
        }
    }
}

#endif // __AVERISERA_MS_OPERATOR_FACTORY_H
