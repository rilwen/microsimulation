/*
(C) Averisera Ltd 2017
*/
#include "state_pension_age_2007.hpp"
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

		static Date get_switch_date(Date::year_type year) {
			return get_switch_date(year, 4);
		}

		static const Date COMMON_TRACK_START = get_switch_date(1959); /*!< First DOB at which females and males share common SPA */

		typedef TimeSeries<Date, Date> series_type;

		static series_type make_linear_series(const Date start_x, const Date end_x, const Date start_y, const Period delta_x, const Period delta_y) {
			Date x = start_x;
			Date y = start_y;
			std::vector<series_type::tvpair_t> data;
			while (x < end_x) {
				data.push_back(series_type::tvpair_t(x, y));
				x = x + delta_x;
				y = y + delta_y;
			}
			return series_type(std::move(data));
		}

		/*! Make scale for linear SPA increase */
		static series_type make_spa_scale(const Date start_dob, const Date end_dob, const Date start_spa) {
			static const Period DELTA_DOB = Period::months(1);
			static const Period DELTA_SPA = Period::months(2);
			return make_linear_series(start_dob, end_dob, start_spa, DELTA_DOB, DELTA_SPA);
		}

		static Date get_date_spa_reached_common(const Date date_of_birth) {
			assert(date_of_birth >= COMMON_TRACK_START);
			// 3 1Y periods of linear increases separated by constant SPA periods
			static const Period incr_period = Period::years(1);
			static const Date s1 = COMMON_TRACK_START;
			static const Date e1 = s1 + incr_period;
			static const Date s2 = get_switch_date(1968);
			static const Date e2 = s2 + incr_period;
			static const Date s3 = get_switch_date(1977);
			static const Date e3 = s3 + incr_period;
			static const series_type scale1(make_spa_scale(s1, e1, get_switch_date(2024, 5)));
			static const series_type scale2(make_spa_scale(s2, e2, get_switch_date(2034, 5)));
			static const series_type scale3(make_spa_scale(s3, e3, get_switch_date(2044, 5)));
			if (date_of_birth < e1) {
				return scale1.padded_value(date_of_birth);
			} else if (date_of_birth < s2) {
				return date_of_birth + Period::years(66);
			} else if (date_of_birth < e2) {
				return scale2.padded_value(date_of_birth);
			} else if (date_of_birth < s3) {
				return date_of_birth + Period::years(67);
			} else if (date_of_birth < e3) {
				return scale3.padded_value(date_of_birth);
			} else {
				return date_of_birth + Period::years(StatePensionAge2007::SPA_AGE_POST);
			}
		}

		Date StatePensionAge2007::get_date_spa_reached_male(const Date date_of_birth) {
			if (date_of_birth >= COMMON_TRACK_START) {
				return get_date_spa_reached_common(date_of_birth);
			} else {
				return date_of_birth + Period::years(MALE_SPA_AGE_PRE);
			}
		}

		Date StatePensionAge2007::get_date_spa_reached_female(const Date date_of_birth) {
			if (date_of_birth >= COMMON_TRACK_START) {
				return get_date_spa_reached_common(date_of_birth);
			} else {
				static const Date female_catchup_start = get_switch_date(1950); /*!< First DOB at which female SPA starts to increase to catch up with male SPA */
				static const Date female_catchup_end = get_switch_date(1955); /*!< DOB at which female SPA stops increasing to catch up with male SPA */
				if (date_of_birth < female_catchup_start) {
					return date_of_birth + Period::years(FEMALE_SPA_AGE_PRE);
				} else if (date_of_birth < female_catchup_end) {
					static const series_type female_catchup_scale(make_spa_scale(female_catchup_start, female_catchup_end, get_switch_date(2010, 5)));
					return female_catchup_scale.padded_value(date_of_birth);
				} else {
					// caught up with men
					return date_of_birth + Period::years(MALE_SPA_AGE_PRE);
				}
			}
		}
	}
}
