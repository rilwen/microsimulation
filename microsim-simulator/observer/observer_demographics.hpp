// (C) Averisera Ltd 2014-2020
#pragma once
#include "../observer.hpp"
#include "microsim-core/person_attributes.hpp"
#include "core/numerical_range.hpp"
#include <vector>

namespace averisera {
	namespace microsim {
		class Contexts;
		class ImmutableContext;
		class Person;
		class Schedule;

        /** Observes births and deaths for each date and value of PersonAttributes */
		class ObserverDemographics: public Observer {
		public:
			typedef uint32_t age_type;
			typedef NumericalRange<age_type> age_range_type;
			typedef std::vector<age_range_type> age_ranges_type;
			/** 
              @param result_saver see Observer
              @param category Category of observed persons
			@param age_ranges Vector of disjoint age ranges to observe
			@param nbr_dates Number of consecutive simulation dates under observation
			@param own_filename_stub Stub for own output filenames. If not empty, save_results will write to new files instead of the provided stream, constructing the filename as "${own_filename_stub}${category}_${values}.csv", where values=population, births or deaths.
			@throw std::domain_error If age_ranges are not all disjoint
			*/
			ObserverDemographics(std::shared_ptr<ObserverResultSaver> result_saver, const std::string& category, const age_ranges_type& age_ranges, size_t nbr_dates, const std::string& own_filename_stub);

			void save_results(std::ostream& os, const ImmutableContext& im_ctx) const override;
            
			typedef int64_t counter_type; /**< Counter type - signed because we want to calculate the differences of them */
			typedef std::vector<counter_type> counters_type; 
			typedef std::pair<age_range_type, PersonAttributes::ethnicity_t> key_type; /**< Map key type: age group is for the person counted unless it's a birth counter, in which case it's the mother's age at birth unless we don't have
																					   a link with mother, in which case it's 0 */
			typedef std::unordered_map<key_type, counters_type> counters_map_type;

			/** Collects counters for female and male persons */
			struct CountersMaps {
				counters_map_type female;
				counters_map_type male;

				/** Requires that all keys are the same and vectors have the same length 
				@throw std::domain_error If this is not the case */
				CountersMaps operator-(const CountersMaps& other) const;

				counters_map_type get_both_sexes() const;
			};

			/** Counts of the whole population */
			const CountersMaps& population_counters() const {
				return _pop_counters;
			}

			/** Counts of births */
			const CountersMaps& birth_counters() const {
				return _birth_counters;
			}

			/** Counts of births to immigrant mothers stored by date of birth */
			const CountersMaps& birth_to_immigrants_by_dob_counters() const {
				return birth_to_immigrants_by_dob_counters_;
			}

			/** Counts of births to immigrant mothers stored by immigration date */
			const CountersMaps& birth_to_immigrants_by_id_counters() const {
				return birth_to_immigrants_by_id_counters_;
			}

			/** Counts of deaths */
			const CountersMaps& death_counters() const {
				return _death_counters;
			}
        protected:
            /** Can be called multiple times on the same date with different batches of persons.
			*/
			void observe_persons(const std::vector<std::shared_ptr<Person>>& persons, const Contexts& ctx);
		private:
			std::string category_;
			age_ranges_type age_ranges_;
			CountersMaps _pop_counters;
			CountersMaps _birth_counters;
			CountersMaps birth_to_immigrants_by_dob_counters_; /**< Count births to immigrant mothers by date of birth */
			CountersMaps birth_to_immigrants_by_id_counters_; /**< Count births to immigrant mothers by immigration date */
			CountersMaps _death_counters;
			size_t _nbr_dates;
			std::string own_filename_stub_;
			counters_type init_counters_;			

			const counters_type& init_counters() const {
				return init_counters_;
			}

			/** Return the range or throw std::runtime_error if no range for this age */
			const age_range_type& get_age_range(double age) const;

			counters_type& get_counters(CountersMaps& maps, PersonAttributes attribs, double age);

			const counters_type& get_counters(const CountersMaps& maps, PersonAttributes attribs, double age) const;

			void save_event_stats(std::ostream& os, const Schedule& sim_schedule, const CountersMaps& maps, const ImmutableContext& im_ctx, const std::string& suffix, bool deltas) const;

			void save_event_stats(std::ostream& os, const Schedule& sim_schedule, const counters_map_type& map, const ImmutableContext& im_ctx, const std::string& suffix, bool deltas) const;

			
		};
	}
}
