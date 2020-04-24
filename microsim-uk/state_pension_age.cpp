/*
(C) Averisera Ltd 2017
*/
#include "state_pension_age.hpp"
#include "core/period.hpp"
#include "core/time_series.hpp"

namespace averisera {
	namespace microsim {
		/*
		Date ranges are [start, end)
		*/

		static Date get_switch_date(Date::year_type year, Date::month_type month) {
			return Date(year, month, 6);
		}

		/*static Date get_switch_date(Date::year_type year) {
			return get_switch_date(year, 4);
		}*/

		static const Date COMMON_TRACK_START = get_switch_date(1953, 12); /*!< First DOB at which females and males share common SPA */

		typedef TimeSeries<Date, Date> date_date_series_type;
		typedef TimeSeries<Date, Period> date_age_series_type;

		/*!
		\param start_x Start of X range (inclusive)
		\param end_x End of X range (exclusive)
		\param start_y Start of Y range (inclusive)
		\param delta_x X range step
		\param delta_y Y range step
		*/
		static date_date_series_type make_linear_series(const Date start_x, const Date end_x, const Date start_y, const Period delta_x, const Period delta_y) {
			Date x = start_x;
			Date y = start_y;
			std::vector<date_date_series_type::tvpair_t> data;
			while (x < end_x) {
				data.push_back(date_date_series_type::tvpair_t(x, y));
				x = x + delta_x;
				y = y + delta_y;
			}
			return date_date_series_type(std::move(data));
		}

		static date_age_series_type make_linear_series(const Date start_x, const Date end_x, const Period start_y, const Period delta_x, const Period delta_y) {
			Date x = start_x;
			Period y = start_y;
			std::vector<date_age_series_type::tvpair_t> data;
			while (x < end_x) {
				data.push_back(date_age_series_type::tvpair_t(x, y));
				x = x + delta_x;
				y = y + delta_y;
			}
			return date_age_series_type(std::move(data));
		}

		static const Period DELTA_DOB = Period::months(1);

		/*! Make scale for linear SPA (date-based) increase 
		\param multiplier How many months the retirement date increases per 1 month of date of birth increase */
		static date_date_series_type make_spa_scale(const Date start_dob, const Date end_dob, const Date start_spa, unsigned int multiplier) {
			assert(multiplier);
			const Period delta_spa = Period::months(multiplier);
			return make_linear_series(start_dob, end_dob, start_spa, DELTA_DOB, delta_spa);
		}

		/*! Make scale for linear SPA (age-based) increase
		\param multiplier How many months the retirement age increases per 1 month of date of birth increase */
		static date_age_series_type make_spa_scale(const Date start_dob, const Date end_dob, const Period start_spa, unsigned int multiplier) {
			assert(multiplier);
			const Period delta_spa = Period::months(multiplier);
			return make_linear_series(start_dob, end_dob, start_spa, DELTA_DOB, delta_spa);
		}

		static Date get_date_spa_reached_common(const Date date_of_birth) {
			assert(date_of_birth >= COMMON_TRACK_START);
			static const Date s65_66 = COMMON_TRACK_START;
			static const Date e65_66 = get_switch_date(1954, 10);
			static const Date s66_67 = get_switch_date(1960, 4);
			static const Date e66_67 = get_switch_date(1961, 3);
			static const Date s67_68 = get_switch_date(1977, 4);
			static const Date e67_68 = get_switch_date(1978, 4);
			static const date_date_series_type scale65_66(make_spa_scale(s65_66, e65_66, get_switch_date(2019, 3), 2));
			static const date_age_series_type scale66_67(make_spa_scale(s66_67, e66_67, Period::years(66) + Period::months(1), 1));
			static const date_date_series_type scale67_68(make_spa_scale(s67_68, e67_68, get_switch_date(2044, 5), 2));
			if (date_of_birth < e65_66) {
				return scale65_66.padded_value(date_of_birth);
			} else if (date_of_birth < s66_67) {
				return date_of_birth + Period::years(66);
			} else if (date_of_birth < e66_67) {
				return date_of_birth + scale66_67.padded_value(date_of_birth);
			} else if (date_of_birth < s67_68) {
				return date_of_birth + Period::years(67);
			} else if (date_of_birth < e67_68) {
				return scale67_68.padded_value(date_of_birth);
			} else {
				return date_of_birth + Period::years(StatePensionAge::SPA_AGE_POST);
			}
		}

		Date StatePensionAge::get_date_spa_reached_male(const Date date_of_birth) {
			if (date_of_birth >= COMMON_TRACK_START) {
				return get_date_spa_reached_common(date_of_birth);
			} else {
				return date_of_birth + Period::years(MALE_SPA_AGE_PRE);
			}
		}

		Date StatePensionAge::get_date_spa_reached_female(const Date date_of_birth) {
			if (date_of_birth >= COMMON_TRACK_START) {
				return get_date_spa_reached_common(date_of_birth);
			} else {
				static const Date female_slow_catchup_start = get_switch_date(1950, 4);
				static const Date female_slow_catchup_end = get_switch_date(1953, 4);
				static const Date female_fast_catchup_end = COMMON_TRACK_START;
				static const date_date_series_type female_slow_catchup_scale(make_spa_scale(female_slow_catchup_start, female_slow_catchup_end, get_switch_date(2010, 5), 2));
				static const date_date_series_type female_fast_catchup_scale(make_spa_scale(female_slow_catchup_end, female_fast_catchup_end, get_switch_date(2016, 7), 4));
				if (date_of_birth < female_slow_catchup_start) {
					return date_of_birth + Period::years(FEMALE_SPA_AGE_PRE);
				} else if (date_of_birth < female_slow_catchup_end) {
					return female_slow_catchup_scale.padded_value(date_of_birth);
				} else {
					assert(date_of_birth < female_fast_catchup_end);
					return female_fast_catchup_scale.padded_value(date_of_birth);
				}
			}
		}
	}
}
