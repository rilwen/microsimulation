#include "perturb_date_of_birth_day.hpp"
#include "../contexts.hpp"
#include "../mutable_context.hpp"
#include "../person_data.hpp"
#include "core/dates.hpp"
#include "core/period.hpp"
#include "core/rng.hpp"

namespace averisera {
	namespace microsim {
		Date PerturbDateOfBirthDay::perturb_date_of_birth(Date date_of_birth, const Contexts& ctx) const {
			const Date d1(date_of_birth.year(), date_of_birth.month(), 1);
			const Date d2(date_of_birth.end_of_month());
			const auto new_day_of_month = static_cast<unsigned short>(ctx.mutable_ctx().rng().next_uniform(static_cast<uint32_t>((d2 - d1).days())));
			return Date(date_of_birth.year(), date_of_birth.month(), new_day_of_month);
		}
	}
}
