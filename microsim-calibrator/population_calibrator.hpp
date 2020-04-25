#pragma once
#include "calibration_types.hpp"
#include "core/data_frame.hpp"
#include "core/numerical_range.hpp"
#include "microsim-core/ethnicity.hpp"

namespace averisera {
	class CSVFileReader;
	namespace microsim {
		class Generation;

		namespace PopulationCalibrator {
			using namespace CalibrationTypes;

			pop_data_type load_population_numbers(CSVFileReader& reader, const Ethnicity::IndexConversions& ic);

			/** Assume columns have disjoint labels */
			void sync_ethnic_group_ranges(std::vector<pop_data_type>& dfs);

			std::vector<pop_data_type> load_population_numbers(const std::vector<std::string>& file_names, CSV::Delimiter delim, const Ethnicity::IndexConversions& ic);

			/**
			@param females Population data from start year census (female)
			@param males Population data from start year census (male)
			@param year Start year for the simulation
			@param max_age Maximum age in the population
			@param ic Converts ethnic group names into indices
			@param sim_start Date when simulation starts
			*/
			std::vector<Generation> make_generations(const pop_data_type& females, const pop_data_type& males, int year, age_type max_age, const Ethnicity::IndexConversions& ic, const Date sim_start);

			std::vector<pop_data_type> subtract_population_numbers(std::vector<pop_data_type> a, std::vector<pop_data_type> b);

			std::vector<pop_data_type> calc_population_increments(const std::vector<pop_data_type>& numbers);
		}
	}
}
