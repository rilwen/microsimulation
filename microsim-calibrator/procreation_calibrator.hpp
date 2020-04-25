#pragma once
#include "calibration_types.hpp"
#include "core/csv.hpp"
#include "microsim-core/conception.hpp"
#include "microsim-core/ethnicity.hpp"
#include "microsim-core/sex.hpp"
#include <string>
#include <vector>

namespace averisera {
	class Distribution;
	namespace microsim {
		class AnchoredHazardCurve;

		namespace ProcreationCalibrator {
			using namespace CalibrationTypes;
			/** Minimum childbearing age for humans */
			const age_type MIN_CHILDBEARING_AGE = 15;

			/** Maximum childbearing age for humans */
			const age_type MAX_CHILDBEARING_AGE = 45; // yes 45, we still have some data for 45

			/** Length of pregnancy as year fraction */
			extern const double PREGNANCY_YEAR_FRACTION;

			/** Load a multiplicity-vs-age distribution series for each year from a CSV-like file which contains the number of multiple births per fixed basis value, indexed by year. All multiple births are assumed to be twins, due to lack of data.
			@param basis For how many births are the multiple births counts given.
			@return distros[i] contains the series of multiplicity distros vs mother's age for i-th year in the file index
			*/
			Conception::mdistr_multi_series_type load_multiplicity_distros(CSVFileReader& reader, double basis);

			/** Load the multiplicity distros and interpolate linearly / extrapolate flat over predefined years 
			@param basis For how many births are the multiple births counts given.
			@return distros[i] contains the series of multiplicity distros vs mother's age for year years[i].
			*/
			Conception::mdistr_multi_series_type load_multiplicity_distros(const std::vector<int>& years, CSVFileReader& reader, double basis);

			/** Load birth numbers, flooring the age groups at MIN_CHILDBEARING_AGE 
			*/
			DataFrame<age_group_type, int> load_birth_numbers(CSVFileReader& reader);

			/*
			Load cohort birth rates (indexed by age group and year of birth) and optionally pad missing data.
			@return Rates normalised to fractions
			*/
			DataFrame<age_group_type, int> load_cohort_birth_rates(CSVFileReader& reader, double basis, bool pad_missing);

			/** Birth rates assumed annual (per year).
			@param birth_rates Indexed by age group and mother's year of birth
			@param period after giving birth during which the woman cannot conceive again, as year fraction
			@param multiplicity_distros 
			@return DataFrame indexed by age group (age at time of giving birth) and year of birth
			*/
			DataFrame<age_group_type, int> calculate_conception_hazard_rates(const DataFrame<age_group_type, int>& birth_rates, const Conception::mdistr_multi_series_type& multiplicity_distros, double post_pregnancy_zero_fertility_year_fraction);

			/**
			@param conception_hazard_rates DataFrame with conception hazard rates indexed by age group (age at time of giving birth) and year of birth
			@return Conception hazard curve for every year of birth in the data frame index
			*/
			std::vector<std::unique_ptr<AnchoredHazardCurve>> calculate_conception_hazard_curves(const DataFrame<age_group_type, int>& conception_hazard_rates);

			/** Load gender rates (fraction of males and females) per year.
			@param basis If > 0, loaded values are assumed to be per basis. If 0, they are assumed to be absolute numbers which have to be normalised. If both MALE and FEMALE numbers are zero, assume 50-50 distribution. */
			DataFrame<Sex, int> load_gender_rates(CSVFileReader& reader, double basis);

			/**
			Load total fertility rate (TFR) values for ethnic groups, indexed by years.
			*/
			DataFrame<Ethnicity::index_range_type, NumericalRange<int>> load_total_fertility_rates_for_ethnic_groups(CSVFileReader& reader, const Ethnicity::IndexConversions& ic);
		}
	}
}
