/*
* (C) Averisera Ltd 2017
*/
#ifndef __AVERISERA_MS_STITCHED_MARKOV_MODEL_CALIBRATOR_HPP
#define __AVERISERA_MS_STITCHED_MARKOV_MODEL_CALIBRATOR_HPP

#include "calibration_types.hpp"
#include "microsim-core/cohort.hpp"
#include "microsim-core/sex.hpp"
#include "microsim-core/stitched_markov_model_with_schedule.hpp"
#include "core/dates.hpp"
#include <string>
#include <utility>
#include <vector>

namespace averisera {
	class CSVFileReader;
	namespace microsim {
		/*!
		Calibrates stitched Markov models for cohorts
		*/
		namespace StitchedMarkovModelCalibrator {
			typedef CalibrationTypes::age_type age_type;
			typedef Date::year_type year_type;

			typedef Cohort::yob_ethn_sex_cohort_type cohort_type;

			/*!
			Build models with 1Y period

			\param min_age Minimum age for which we model the variable
			\param min_year_of_birth lowest year of birth considered
			\param min_year Start of simulation year
			\param max_year Maximum calendar year (end of simulation), inclusive
			\param dim Dimension of the Markov process
			\param month Month in the year when each model will start
			\param day Day of the month when each model will start

			\tparam S Markov process state type
			*/
			template <class S> void calibrate_annual_models(age_type min_age, year_type min_year_of_birth, year_type min_year, year_type max_year, 
				S dim, Date::month_type month, Date::day_type day, CSVFileReader& reader,
				std::vector<StitchedMarkovModelWithSchedule<S>>& models,
				std::vector<cohort_type>& cohorts
				);
		}
	}
}

#endif // __AVERISERA_MS_STITCHED_MARKOV_MODEL_CALIBRATOR_HPP
