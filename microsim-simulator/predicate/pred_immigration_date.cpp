#include "pred_immigration_date.hpp"
#include "core/preconditions.hpp"
#include "../person.hpp"

namespace averisera {
	namespace microsim {
		PredImmigrationDate::PredImmigrationDate(Date from, Date to, bool allow_non_immigrants, bool require_alive)
			: from_(from), to_(to), allow_non_immigrants_(allow_non_immigrants), require_alive_(require_alive) {
			check_that(from <= to, "PredImmigrationDate: from date must be <= to date");
		}

		bool PredImmigrationDate::select(const Person& obj, const Contexts& contexts) const {
			// TODO: factor out this construction into a PredPerson abstract class
			if (select_out_of_context(obj)) {
				return (!require_alive_) || obj.is_alive(contexts.asof());				
			} else {
				return false;
			}
		}

		bool PredImmigrationDate::select_out_of_context(const Person& obj) const {
			if (obj.immigration_date().is_not_a_date()) {
				return allow_non_immigrants_;
			} else {
				return (obj.immigration_date() >= from_) && (obj.immigration_date() < to_);
			}
		}

		bool PredImmigrationDate::active(Date date) const {
			// Before from_ date, no immigrant can qualify
			return allow_non_immigrants_ || date >= from_;
		}
		
		void PredImmigrationDate::print(std::ostream& os) const {
			os << "ImmigrationDate(" << from_ << ", " << to_ << ", " << allow_non_immigrants_ << ", " << require_alive_ << ")";
		}

		Predicate<Person>* PredImmigrationDate::clone() const {
			return new PredImmigrationDate(from_, to_, allow_non_immigrants_, require_alive_);
		}
	}
}
