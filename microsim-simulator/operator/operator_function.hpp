#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "../history_generator_simple.hpp"
#include "../history_user_simple.hpp"
#include "../operator_individual.hpp"
#include "microsim-core/cohort.hpp"
#include "microsim-core/ethnicity.hpp"
#include "microsim-core/schedule.hpp"
#include <string>

namespace averisera {
	class Distribution;
	namespace microsim {
		/**
		Operator which converts variable X to variably Y (double to double)
		*/
		template <class T> class OperatorFunction : public OperatorIndividual<T> {
		public:
			typedef std::function<double(double)> function_type;
			/**
			@param input_name Input variable name
			@param output_name Output variable_name
			@param predicate Operator predicate
			@param functions functions[i] is to be applied to i-th date in the custom or default (i.e. simulation) schedule
			@param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule.
			@param output_history_factory HistoryFactory for output variable histories
			@throw std::domain_error If predicate or any of the functions is null. If input_name or output_name is empty.
			@throw std::out_of_range If schedule is not null and there is less functions than dates in the schedule.
			*/
			OperatorFunction(const std::string& input_name,
				const std::string& output_name,
				std::shared_ptr<const Predicate<T>> predicate,
				const std::vector<function_type>& functions,
				std::unique_ptr<Schedule>&& schedule,
				HistoryFactory::factory_t output_history_factory
				);

			void apply(const std::shared_ptr<T>& obj, const Contexts& contexts) const override;

			const Predicate<T>& predicate() const override {
				return *predicate_;
			}

			bool active(Date date) const override {
				// operator is active on date if the date is in the schedule or schedule is null
				return Operator<T>::active(schedule_, date);
			}

			const typename HistoryGenerator<T>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const typename HistoryUser<T>::use_reqvec_t& user_requirements() const override {
				return hist_use_.user_requirements();
			}

			const std::string& name() const override {
				return operator_name_;
			}

			
		private:
			std::string input_name_;
			std::string output_name_;
			HistoryGeneratorSimple<T> hist_gen_;
			HistoryUserSimple<T> hist_use_;
			std::shared_ptr<const Predicate<T>> predicate_;
			std::vector<function_type> functions_;
			std::unique_ptr<Schedule> schedule_;
			std::string operator_name_;
		};

		/**
		Build a set of operators for a number of cohorts of Person objects

		@param ethnic_groupings Map ethnic group names to sets of ethnicity indices
		@param functions Vector of vectors of functions; if functions[i].empty(), we skip i-th cohort
		@param period_years Distance in years between function invocations
		@param cohorts Cohort definitions (using the names from ethnic_groupings)
		@param start_dates Start date for every schedule
		@return Vector of operators, for every cohort which had functions
		@throw std::domain_error If cohorts.size() != functions.size()
		*/
		std::vector<std::unique_ptr<Operator<Person>>> build_operator_function_for_cohorts(const std::map<std::string, Ethnicity::index_set_type>& ethnic_groupings,
			const std::string& input_name,
			const std::string& output_name,
			const std::vector<std::vector<OperatorFunction<Person>::function_type>>& functions,
			unsigned int period_years,
			const std::vector<Cohort::yob_ethn_sex_cohort_type>& cohorts,
			const std::vector<Date>& start_dates,
			HistoryFactory::factory_t output_history_factory);
	}
}
