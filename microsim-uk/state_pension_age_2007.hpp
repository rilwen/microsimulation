#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "microsim-core/sex.hpp"
#include "core/dates.hpp"

namespace averisera {
	namespace microsim {
		/** Describes state pension age (SPA) before and after 2007 reform */
		struct StatePensionAge2007 {
			/** Male SPA prior to 2007 reform */
			static const unsigned int MALE_SPA_AGE_PRE = 65;

			/** Female SPA prior to 2007 reform */
			static const unsigned int FEMALE_SPA_AGE_PRE = 60;

			/** Male/female SPA after 2007 reform completes */
			static const unsigned int SPA_AGE_POST = 68;

			static constexpr unsigned int get_spa_age(Sex sex, bool after_reform) {
				return after_reform ? SPA_AGE_POST : (sex == Sex::MALE ? MALE_SPA_AGE_PRE : FEMALE_SPA_AGE_PRE);
			}

			/** SPA date resulting from the reform including the transitional cohorts (males) */
			static Date get_date_spa_reached_male(Date date_of_birth);

			/** SPA date resulting from the reform including the transitional cohorts (females) */
			static Date get_date_spa_reached_female(Date date_of_birth);

			/** SPA date resulting from the reform including the transitional cohorts */
			static Date get_date_spa_reached(Sex sex, Date date_of_birth) {
				if (sex == Sex::MALE) {
					return get_date_spa_reached_male(date_of_birth);
				} else {
					return get_date_spa_reached_female(date_of_birth);
				}
			}
		};
	}
}
