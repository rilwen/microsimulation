#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "microsim-core/sex.hpp"
#include "core/dates.hpp"
#include "core/daycount.hpp"
#include "core/period.hpp"
#include "core/time_series.hpp"
#include <vector>

namespace averisera {
	namespace testing {
		typedef TimeSeries<Date, double> spa_series_type;

		/** Construct a series of SPA as function of retirement date, assuming monotonic increase of SPA per cohort */
		template <class SPA> spa_series_type make_spa_at_retirement_date_series(const Date start, const Date end, const microsim::Sex sex) {
			std::vector<spa_series_type::tvpair_t> data;
			Date dob = start - Period::years(100);
			static const Period delta(PeriodType::DAYS, 1);
			const auto dc = Daycount::YEAR_FRACT();
			while (dob <= end) {
				const Date retirement_date = SPA::get_date_spa_reached(sex, dob);
				if (retirement_date >= start) {
					if (retirement_date > end) {
						break;
					} else {
						const double spa = dc->calc(dob, retirement_date);
						if (data.empty() || data.back().first < retirement_date) {
							data.push_back(spa_series_type::tvpair_t(retirement_date, spa));
						} else {
							if (retirement_date == data.back().first) {
								data.back().second = std::min(spa, data.back().second);
							} else {
								throw std::runtime_error("Retirement dates not monotonic");
							}
						}
					}
				}
				dob = dob + delta;
			}
			return spa_series_type(std::move(data));
		}
	}
}
