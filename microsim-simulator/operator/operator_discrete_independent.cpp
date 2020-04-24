/*
(C) Averisera Ltd 2017
*/
#include "core/log.hpp"
#include "microsim-core/schedule_definition.hpp"
#include "../contexts.hpp"
#include "../feature.hpp"
#include "../immutable_context.hpp"
#include "../person.hpp"
#include "operator_discrete_independent.hpp"

namespace averisera {
	namespace microsim {
		template <class S> OperatorDiscreteIndependent<S>::OperatorDiscreteIndependent(const std::string& feature, const StitchedMarkovModelWithSchedule<S>& model,
			const std::shared_ptr<const Predicate<Person>> predicate, 
			const bool initialise, 
			const std::string& percentile_variable_name,
			std::unique_ptr<Schedule>&& schedule, 
			const bool store_percentiles_as_floats)
			: OperatorIndividual<Person>(true, Feature::from_names(get_provided_features(feature, percentile_variable_name, initialise)), Feature::from_names(get_required_features(feature, initialise))),
			hist_gen_(build_history_generator(feature, percentile_variable_name, predicate, store_percentiles_as_floats)),
			hist_user_(build_history_user(feature, percentile_variable_name, predicate)),
			model_(model), 
			pred_(predicate), 
			name_(std::string("DiscreteIndependent_") + feature),
			feature_(feature),
			initialise_(initialise),
			percentile_variable_name_(percentile_variable_name)
		{			
			check_that(pred_ != nullptr, "OperatorDiscreteIndependent: null predicate");
			schedule_ = std::move(schedule);
			LOG_TRACE() << "OperatorDiscreteIndependent: constructor";
		}

		template <class S> HistoryGeneratorSimple<Person> OperatorDiscreteIndependent<S>::build_history_generator(const std::string& feature, const std::string& percentile_variable_name, std::shared_ptr<const Predicate<Person>> predicate, const bool store_percentiles_as_floats) {			
			auto feature_history_factory = HistoryFactory::SPARSE<state_type>();
			if (percentile_variable_name.empty()) {
				HistoryGeneratorSimple<Person> generator(feature, feature_history_factory, predicate);
				LOG_TRACE() << "OperatorDiscreteIndependent::build_history_generator: built history generator with feature " << feature << " and predicate " << predicate->as_string();
				return generator;
			} else {
				const std::vector<std::string> names({ feature, percentile_variable_name });
				const auto percentile_history_factory = store_percentiles_as_floats ? HistoryFactory::SPARSE<float>() : HistoryFactory::SPARSE<double>();
				const std::vector<HistoryGeneratorSimple<Person>::factory_t> factories({ feature_history_factory, percentile_history_factory });
				HistoryGeneratorSimple<Person> generator(names.begin(), names.end(), factories.begin(), factories.end(), predicate);
				LOG_TRACE() << "OperatorDiscreteIndependent::build_history_generator: built history generator with names " << names << " and predicate " << predicate->as_string();
				return generator;
			}
		}

		template <class S> HistoryUserSimple<Person> OperatorDiscreteIndependent<S>::build_history_user(const std::string& feature, const std::string& percentile_variable_name, std::shared_ptr<const Predicate<Person>> predicate) {
			if (percentile_variable_name.empty()) {
				HistoryUserSimple<Person> user(feature);
				LOG_TRACE() << "OperatorDiscreteIndependent::build_history_user: built history user with feature " << feature;
				return user;
			} else {
				std::vector<std::string> names({ feature, percentile_variable_name });
				HistoryUserSimple<Person> user(std::move(names));
				LOG_TRACE() << "OperatorDiscreteIndependent::build_history_user: built history user with names " << user.user_requirements();
				return user;
			}
		}

		template <class S> void OperatorDiscreteIndependent<S>::apply(const std::shared_ptr<Person>& person, const Contexts& ctx) const {
			if (schedule_) {
				assert(ctx.immutable_ctx().schedule().contains(*schedule_));
			}

			const auto hist_idx = person->get_variable_index(ctx, feature_);
			History& history = person->history(hist_idx);
			const Date asof = ctx.asof();
			if (initialise_ && history.empty()) {
				// draw new 
				if (asof >= model_.start_date()) {
					const double u = ctx.mutable_ctx().rng().next_uniform();
					const state_type k = model_.draw_future_state(asof, u);
					//LOG_TRACE() << "OperatorDiscreteIndependent(" << feature_ << "): initialised history with " << k << " as of " << asof;
					if (!percentile_variable_name_.empty()) {
						// u is the percentile
						append_percentile(person, ctx, asof, u);
					}
					history.append_int(asof, k);
				}
			} else {
				const Date last_date = history.last_date();
				if (last_date + model_.period() <= asof) {
					// time for new value!
					const state_type k = static_cast<state_type>(history.last_as_int());
					const double u = ctx.mutable_ctx().rng().next_uniform();
					state_type l;
					if (percentile_variable_name_.empty()) {
						l = model_.draw_next_state(k, last_date, u);
					} else {
						const auto next_state_and_percentile = model_.draw_next_state_and_percentile(k, last_date, u);
						l = next_state_and_percentile.first;
						append_percentile(person, ctx, asof, next_state_and_percentile.second);
					}
					history.append_int(asof, l);
				}
			}
		}

		template <class S> void OperatorDiscreteIndependent<S>::append_percentile(const std::shared_ptr<Person>& person, const Contexts& ctx, Date asof, double p) const {
			assert(!percentile_variable_name_.empty());
			History& percentile_history = person->history(ctx.immutable_ctx(), percentile_variable_name_);
			percentile_history.append(asof, p);
		}

		template <class S> std::vector<std::unique_ptr<Operator<Person>>> OperatorDiscreteIndependent<S>::build_for_cohorts(const std::map<std::string, Ethnicity::index_set_type>& ethnic_groupings,
			const std::string& category_variable_name,
			const bool do_initialisation,
			const std::string& percentile_variable_name,
			const std::vector<StitchedMarkovModelWithSchedule<S>>& models,
			const std::vector<Cohort::yob_ethn_sex_cohort_type>& cohorts,
			const std::vector<Date>& start_dates,
			const unsigned int period_years,
			const Date end_date,
			const bool store_percentiles_as_floats)
		{
			const size_t n = models.size();
			std::vector<std::unique_ptr<Operator<Person>>> operators;
			check_equals(n, cohorts.size(), "OperatorDiscreteIndependent::build_for_cohorts: number of models does not match the number of cohorts");
			operators.reserve(n);
			std::map<std::string, std::shared_ptr<const Predicate<Person>>> ethnic_predicates;
			for (const auto& kv : ethnic_groupings) {
				ethnic_predicates[kv.first] = PredicateFactory::make_ethnicity(kv.second, true);
			}
			LOG_DEBUG() << "OperatorDiscreteIndependent::build_for_cohorts: generated " << ethnic_predicates.size() << " ethnic predicates";
			const auto period = Period::years(period_years);
			for (size_t i = 0; i < n; ++i) {
				const auto& cohort = cohorts[i];
				// don't specify min_age, operator will act on all persons born in a year selected by the cohort data
				const std::shared_ptr<const Predicate<Person>> predicate(PredicateFactory::make_cohort(ethnic_predicates, cohort, true, 0));				
				LOG_TRACE() << "OperatorDiscreteIndependent::build_for_cohorts: built predicate for cohort " << cohort << ": " << predicate->as_string();
				const Date start_date = start_dates[i];
				std::unique_ptr<Schedule> schedule = std::make_unique<Schedule>(ScheduleDefinition(start_date, end_date, period));
				std::unique_ptr<Operator<Person>> operator_ptr(new OperatorDiscreteIndependent<S>(category_variable_name, models[i], predicate, do_initialisation, percentile_variable_name, std::move(schedule), store_percentiles_as_floats));
				LOG_TRACE() << "OperatorDiscreteIndependent::build_for_cohorts: built operator " << operator_ptr->name() << " for cohort " << cohort;
				operators.push_back(std::move(operator_ptr));
				LOG_TRACE() << "OperatorDiscreteIndependent::build_for_cohorts: pushed back operator for cohort " << cohort;
			}
			LOG_DEBUG() << "OperatorDiscreteIndependent::build_for_cohorts: built " << operators.size() << " operators";
			return operators;
		}

		template <class S> std::vector<std::string> OperatorDiscreteIndependent<S>::get_provided_features(const std::string& feature, const std::string& percentile_variable_name, const bool initialise) {
			std::vector<std::string> provided_features;
			provided_features.push_back(feature);
			if (initialise) {
				provided_features.push_back(get_initialisation_feature(feature));
			}
			if (!percentile_variable_name.empty()) {
				provided_features.push_back(percentile_variable_name);
			}
			LOG_TRACE() << "OperatorDiscreteIndependent::get_provided_features: returning " << provided_features;
			return provided_features;
		}

		template <class S> std::vector<std::string> OperatorDiscreteIndependent<S>::get_required_features(const std::string& feature, const bool initialise) {
			std::vector<std::string> required_features;
			if (!initialise) {
				required_features.push_back(get_initialisation_feature(feature));
			}
			LOG_TRACE() << "OperatorDiscreteIndependent::get_required_features: returning " << required_features;
			return required_features;
		}

		template <class S> std::string OperatorDiscreteIndependent<S>::get_initialisation_feature(const std::string& main_feature) {
			return main_feature + "_Init";
		}

		template class OperatorDiscreteIndependent<uint8_t>;
		template class OperatorDiscreteIndependent<uint16_t>;
		template class OperatorDiscreteIndependent<uint32_t>;
	}
}
