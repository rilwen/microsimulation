// (C) Averisera Ltd 2014-2020
#include "perturb_date_of_birth_month.hpp"
#include "../contexts.hpp"
#include "../mutable_context.hpp"
#include "../person_data.hpp"
#include "core/dates.hpp"
#include "core/period.hpp"
#include "core/rng.hpp"

namespace averisera {
	namespace microsim {
		Date PerturbDateOfBirthMonth::perturb_date_of_birth(Date date_of_birth, const Contexts& ctx) const {
			const auto new_month = static_cast<unsigned short>(1 + ctx.mutable_ctx().rng().next_uniform(11u));
            const Period delta(PeriodType::MONTHS, new_month - date_of_birth.month());
			return date_of_birth + delta; // use Period so that we can safely change 30th June into 28th February etc.
		}
	}
}
