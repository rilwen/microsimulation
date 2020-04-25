#pragma once
#include "perturb_date_of_birth.hpp"

namespace averisera {
	namespace microsim {
		struct PersonData;

		/** Change the month part of date of birth to a uniformly random one within the same year. */
		class PerturbDateOfBirthMonth : public PerturbDateOfBirth {
        public:
            /** @see PerturbDateOfBirth */
            PerturbDateOfBirthMonth(bool shift_history_dates)
                : PerturbDateOfBirth(shift_history_dates) {}
		private:
			Date perturb_date_of_birth(Date date_of_birth, const Contexts& ctx) const override;
		};
	}
}
