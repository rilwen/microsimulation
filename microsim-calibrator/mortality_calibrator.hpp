// (C) Averisera Ltd 2014-2020
#pragma once
#include "calibration_types.hpp"
#include "rate_calibrator.hpp"
#include "core/dates_fwd.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace averisera {
	class CSVFileReader;
	template <class C, class I, class V> class DataFrame;
	namespace microsim {
		class AnchoredHazardCurve;
		template <class G> struct MortalityRate;

		/** Methods to calibrate mortality curves from data 
		*/
		namespace MortalityCalibrator {
			using namespace CalibrationTypes;
			typedef RateCalibrator::year_t year_t;

			/** Calculate a mortality curve for each cohort for which we have at least 1 mortality rate provided in data.
			Mortality curves for cohorts born before the first cohort with data are back-filled by translating the first available curve to the 1 January of the new year of birth.
			@param mortality_rates Vector of mortality rates observed in given years and for given age groups
			@param min_year_of_birth Minimum year of birth (floored by Date::MIN_YEAR)
			@param min_year_of_birth Maximum year of birth (ceiled by Date::MAX_YEAR)
			@throw std::runtime_error If age group in a mortality rate has boundaries in wrong order or if there are duplicate data for (year, year of birth) pair.
			@return Vector of mortality curves 
			*/
			std::vector<std::unique_ptr<AnchoredHazardCurve>> calc_mortality_curves(const std::vector<MortalityRate<age_group_type>>& mortality_rates, Date::year_type min_year_of_birth, Date::year_type max_year_of_birth);

			/** Calculate cohort mortality curves based on mortality rates indexed by year and age group. Cohorts which are missing
			mortality rates for some age groups copy them from latest / first available, depending on whether they are too young
			or too old.
			@param rates Matrix of mortality rates indexed by years (rows) and age groups (columns). 
			@param years Vector of years, strictly increasing 
			@param age_groups Vector of age groups, not overlapping.
			@param ref_pop_size Reference population size (rates(r, c) is the number of people out of 
			@param min_year_of_birth Minimum year of birth (floored by Date::MIN_YEAR)
			@param max_year_of_birth Maximum year of birth (ceiled by Date::MAX_YEAR)
			@throw std::domain_error If rates has wrong dimensions. If years or age_groups is empty.
			@throw std::runtime_error If age group in a mortality rate has boundaries in wrong order. If age groups are overlapping.
			*/
			std::vector<std::unique_ptr<AnchoredHazardCurve>> calc_mortality_curves(const DataFrame<age_group_type, int, double>& rates, Date::year_type min_year_of_birth, Date::year_type max_year_of_birth);

			/** Calculate mortality curves using mortality rates read from a CSV file.
			@param reader CSV file reader CSV file reader for mortality rates in age groups in given years
			@param min_year_of_birth Minimum year of birth (floored by Date::MIN_YEAR)
			@param max_year_of_birth Maximum year of birth (ceiled by Date::MAX_YEAR)
			*/
			std::vector<std::unique_ptr<AnchoredHazardCurve>> calc_mortality_curves(CSVFileReader& reader, Date::year_type min_year_of_birth, Date::year_type max_year_of_birth);

			/** Calculate mortality curves using death counts and age group sizes read from CSV files.
			@param death_reader CSV file reader CSV file reader for death counts in age groups and given years
			@param group_size_reader CSV file reader CSV file reader for age group sizes in given years
			@param min_year_of_birth Minimum year of birth (floored by Date::MIN_YEAR)
			@param max_year_of_birth Maximum year of birth (ceiled by Date::MAX_YEAR)
			*/
			std::vector<std::unique_ptr<AnchoredHazardCurve>> calc_mortality_curves(CSVFileReader& death_reader, CSVFileReader& group_size_reader, Date::year_type min_year_of_birth, Date::year_type max_year_of_birth);
		}
	}
}
