/*
(C) Averisera Ltd 2017
*/

#pragma once
#include "sex.hpp"
#include "core/dates.hpp"
#include <string>
#include <utility>

namespace averisera {
	namespace microsim {
		/*! Various ways of defining a cohort */
		namespace Cohort {
			typedef Date::year_type year_type;
			typedef std::tuple<year_type, std::string, Sex> yob_ethn_sex_cohort_type; // (year of birth, ethnic grouping, sex)
		}
	}
}
