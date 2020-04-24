/*
(C) Averisera Ltd 2017
*/

#include "operator_function.hpp"
#include "../immutable_context.hpp"
#include "../history/immutable_history_truncated.hpp"
#include "../person.hpp"
#include "microsim-core/schedule_definition.hpp"
#include "core/distribution.hpp"

namespace averisera {
	namespace microsim {
		template <class T> OperatorFunction<T>::OperatorFunction(const std::string& input_name,
			const std::string& output_name,
			std::shared_ptr<const Predicate<T>> predicate,
			const std::vector<function_type>& functions,
			std::unique_ptr<Schedule>&& schedule,
			HistoryFactory::factory_t output_history_factory
			)
			: OperatorIndividual<T>(true, FeatureUser<Feature>::feature_set_t({ output_name }), FeatureUser<Feature>::feature_set_t({ input_name })),
			input_name_(input_name),
			output_name_(output_name),
			hist_gen_(output_name, output_history_factory, predicate),
			hist_use_(input_name),
			predicate_(predicate),
			functions_(functions),
			operator_name_(boost::str(boost::format("%s->%s") % input_name % output_name)) {
			if (!predicate) {
				throw std::domain_error("OperatorFunction: null predicate");
			}
			if (std::any_of(functions.begin(), functions.end(), [](const function_type& fun) { return !fun; })) {
				throw std::domain_error("OperatorFunction: one or more functions is null");
			}
			if (input_name.empty()) {
				throw std::domain_error("OperatorFunction: empty percentile name");
			}
			if (output_name.empty()) {
				throw std::domain_error("OperatorFunction: empty variable name");
			}
			if (schedule && schedule->nbr_dates() > functions_.size()) {
				throw std::out_of_range("OperatorFunction: not enough functions for custom schedule");
			}
			schedule_ = std::move(schedule);
		}

		template <class T> void OperatorFunction<T>::apply(const std::shared_ptr<T>& obj, const Contexts& contexts) const {
			if (!schedule_) {
				// custom schedule would have been checked in constructor
				if (contexts.immutable_ctx().schedule().nbr_dates() > functions_.size()) {
					throw std::out_of_range("OperatorFunction: not enough functions");
				}
			} else {
				assert(contexts.immutable_ctx().schedule().contains(*schedule_));
			}

			const Date asof = contexts.asof();
			const Schedule::index_t idx = schedule_ ? schedule_->index(asof) : contexts.asof_idx();
			const function_type& f = functions_[idx];

			const ImmutableHistory& input_history_orig = obj->history(contexts.immutable_ctx(), input_name_);
			const ImmutableHistoryTruncated input_history(input_history_orig, asof);
			if (input_history.empty()) {
				LOG_ERROR() << "OperatorFunction::apply: input " << input_name_ << " history empty as of " << asof << " while applying operator " << operator_name_ << " with predicate " << predicate_->as_string();
				throw std::domain_error("OperatorFunction: input history empty");
			}
			const double x = input_history.as_double(asof);
			if (std::isnan(x)) {
				const Date last_available_date = input_history.last_date(asof);
				const double last_x = input_history.last_as_double(asof);
				LOG_WARN() << "OperatorFunction:apply: input " << input_name_ << " history has no value as of " << asof << " while applying operator " << operator_name_ << " with predicate " << predicate_->as_string() << "; last available date is " << last_available_date << " with value " << last_x;
				// skip it, will be initialised later
				//throw std::domain_error("OperatorFunction: no input value at this date");
			} else {
				const double y = f(x);
				History& output_history = obj->history(contexts.immutable_ctx(), output_name_);
				const bool empty_output_history = output_history.empty();
				if (!empty_output_history && (output_history.last_date() == asof)) {
					const double old_y = output_history.last_as_double();
					if (old_y != y) {
						LOG_WARN() << "OperatorFunction::apply: " << input_name_ << " history already has a value " << " as of " << asof << ", correcting to " << y;
						output_history.append_or_correct(asof, y);
					} else {
						LOG_WARN() << "OperatorFunction::apply: " << input_name_ << " history already has value " << y << " as of " << asof;
					}					
				} else {
					// trigger an error if we have a value PAST asof, as clearly something has gone wrong in this case...
					output_history.append(asof, y);
				}
			}
		}

		template class OperatorFunction<Actor>;
		template class OperatorFunction<Person>;

		std::vector<std::unique_ptr<Operator<Person>>> build_operator_function_for_cohorts(const std::map<std::string, Ethnicity::index_set_type>& ethnic_groupings,
			const std::string& input_name,
			const std::string& output_name,
			const std::vector<std::vector<OperatorFunction<Person>::function_type>>& functions,
			const unsigned int period_years,
			const std::vector<Cohort::yob_ethn_sex_cohort_type>& cohorts,
			const std::vector<Date>& start_dates,
			HistoryFactory::factory_t output_history_factory)
		{
			const size_t n = functions.size();
			std::vector<std::unique_ptr<Operator<Person>>> operators;
			check_equals(n, cohorts.size(), "build_operator_function_for_cohorts: different number of cohorts and functions vectors");
			check_equals(n, start_dates.size(), "build_operator_function_for_cohorts: different number of cohorts and start dates vectors");
			operators.reserve(n);
			std::map<std::string, std::shared_ptr<const Predicate<Person>>> ethnic_predicates;
			for (const auto& kv : ethnic_groupings) {
				ethnic_predicates[kv.first] = PredicateFactory::make_ethnicity(kv.second, true);
			}
			const auto period = Period::years(period_years);
			for (size_t i = 0; i < n; ++i) {
				const auto& cohort = cohorts[i];
				if (functions[i].empty()) {
					LOG_INFO() << "build_operator_function_for_cohorts: skipping cohort " << cohort << " because no functions provided";
					continue;
				}
				// don't specify min_age, operator will act on all persons born in a year selected by the cohort data
				std::shared_ptr<const Predicate<Person>> predicate(PredicateFactory::make_cohort(ethnic_predicates, cohort, true, 0));
				const Date start_date = start_dates[i];
				const Date end_date = start_date + Period::years(MathUtils::safe_cast<int>(functions[i].size() - 1));
				std::unique_ptr<Schedule> schedule = std::make_unique<Schedule>(ScheduleDefinition(start_date, end_date, period));
				operators.push_back(std::make_unique<OperatorFunction<Person>>(input_name, output_name, predicate, functions[i],
					std::move(schedule), output_history_factory));
				LOG_DEBUG() << "build_operator_function_for_cohorts: created a OperatorFunction " << operators.back()->name() << " for cohort " << cohort << " acting from " << start_date << " to " << end_date << " every " << period_years << " years";
			}
			return operators;
		}
	}
}
