#ifndef __AVERISERA_MS_OPERATOR_DISCRETE_INDEPENDENT_HPP
#define __AVERISERA_MS_OPERATOR_DISCRETE_INDEPENDENT_HPP
/*
(C) Averisera Ltd 2017
*/

#include "../history_generator_simple.hpp"
#include "../history_user_simple.hpp"
#include "../operator_individual.hpp"
#include "microsim-core/cohort.hpp"
#include "microsim-core/stitched_markov_model_with_schedule.hpp"
#include <string>

namespace averisera {
	namespace microsim {
		class Person;

		/** Operator modelling a state variable X which is discrete and independent from other variables.

		@tparam S State class (unsigned int type)
		*/ 
		template <class S = unsigned int> class OperatorDiscreteIndependent : public OperatorIndividual<Person> {
		public:
			/**
			@param feature Feature provided by the operator (e.g. "BMI"); equal to the variable X name
			@param model One or more discrete-time Markov models stitched together
			@param initialise Whether to initialise the variable (or leave this to another operator)
			@param percentile_variable_name Variable name for the percentiles of X (empty if no percentiles required)
			@param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule.
			@param store_percentiles_as_floats Whether to store percentiles as floats (true) or doubles (false)
			@throw std::domain_error If predicate is null
			*/
			OperatorDiscreteIndependent(const std::string& feature, const StitchedMarkovModelWithSchedule<S>& model, std::shared_ptr<const Predicate<Person>> predicate, bool initialise, const std::string& percentile_variable_name,
				std::unique_ptr<Schedule>&& schedule, bool store_percentiles_as_floats);

			void apply(const std::shared_ptr<Person>& person, const Contexts& contexts) const override;

			const Predicate<Person>& predicate() const override {
				return *pred_;
			}

			const std::string& name() const override {
				return name_;
			}

			const HistoryGenerator<Person>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const HistoryUser<Person>::use_reqvec_t& user_requirements() const override {
				return hist_user_.user_requirements();
			}

			bool active(Date date) const override {
				// operator is active on date if the date is in the schedule or schedule is null
				return Operator<Person>::active(schedule_, date);
			}

			/**
			@param ethnic_groupings Map ethnic group names to sets of ethnicity indices
			@param category_variable_name Feature name
			@param percentile_variable_name Optional percentile variable name (empty if percentiles not required)
			@param models Models for each cohorts
			@param cohorts Cohort definitions (using the names from ethnic_groupings)
			@param start_dates Start date for every schedule
			@param period_years Distance in years between operator applications
			@param end_date Last date when any operator will be invoked
			@param store_percentiles_as_floats Whether to store percentiles as floats (true) or doubles (false)
			*/
			static std::vector<std::unique_ptr<Operator<Person>>> build_for_cohorts(const std::map<std::string, Ethnicity::index_set_type>& ethnic_groupings,
				const std::string& category_variable_name,
				bool do_initialisation,
				const std::string& percentile_variable_name,
				const std::vector<StitchedMarkovModelWithSchedule<S>>& models,
				const std::vector<Cohort::yob_ethn_sex_cohort_type>& cohorts,
				const std::vector<Date>& start_dates,
				unsigned int period_years,
				Date end_date,
				bool store_percentiles_as_floats);

			/** Get the name of the feature responsible for initialising the main feature (discrete independent variable) 
			This function should be used if another operator handles the initialisation 
			*/
			static std::string get_initialisation_feature(const std::string& main_feature);
		private:
			HistoryGeneratorSimple<Person> hist_gen_;
			HistoryUserSimple<Person> hist_user_;
			StitchedMarkovModelWithSchedule<S> model_;
			std::shared_ptr<const Predicate<Person>> pred_;
			std::string name_;
			std::string feature_;
			bool initialise_;
			std::string percentile_variable_name_;
			std::unique_ptr<Schedule> schedule_;

			typedef typename StitchedMarkovModelWithSchedule<S>::state_type state_type;

			static HistoryGeneratorSimple<Person> build_history_generator(const std::string& feature, const std::string& percentile_variable_name, std::shared_ptr<const Predicate<Person>> predicate, bool store_percentiles_as_floats);
			static HistoryUserSimple<Person> build_history_user(const std::string& feature, const std::string& percentile_variable_name, std::shared_ptr<const Predicate<Person>> predicate);

			void append_percentile(const std::shared_ptr<Person>& person, const Contexts& ctx, Date asof, double p) const;

			static std::vector<std::string> get_provided_features(const std::string& feature, const std::string& percentile_variable_name, bool initialise);

			static std::vector<std::string> get_required_features(const std::string& feature, bool initialise);
		};
	}
}

#endif // __AVERISERA_MS_OPERATOR_DISCRETE_ORDINAL_INDEPENDENT_HPP
