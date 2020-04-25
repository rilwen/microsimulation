#pragma once
#include "data_perturbation_individual.hpp"
#include "core/dates_fwd.hpp"

namespace averisera {
	namespace microsim {
		struct PersonData;

		/** Perturb date of birth. If conception date if given, move it by the same number of days. */
		class PerturbDateOfBirth : public DataPerturbation<PersonData> {
        public:
            /** @param shift_history_dates Whether to move the dates in histories by (new birth date - old birth date) offset. 
             */
            PerturbDateOfBirth(bool shift_history_dates)
                : _shift_history_dates(shift_history_dates), _avoid_linked(true) {}
            
            void apply(std::vector<PersonData>& datas, const Contexts& ctx) const override;
		private:
			virtual Date perturb_date_of_birth(Date date_of_birth, const Contexts& ctx) const = 0;

            bool _shift_history_dates;
            bool _avoid_linked; /**< If true, do not perturb birth dates of persons linked to their parents */
		};
	}
}
