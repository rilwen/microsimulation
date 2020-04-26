// (C) Averisera Ltd 2014-2020
#pragma once
#include "perturb_date_of_birth.hpp"

namespace averisera {
    namespace microsim {
		struct PersonData;

		/** Change the day part of date of birth to a uniformly random one within the same month. */
		class PerturbDateOfBirthDay : public PerturbDateOfBirth {
        public:
            /** @see PerturbDateOfBirth */
            PerturbDateOfBirthDay(bool shift_history_dates)
                : PerturbDateOfBirth(shift_history_dates) {}
		private:
			Date perturb_date_of_birth(Date date_of_birth, const Contexts& ctx) const override;
		};
    }
}
