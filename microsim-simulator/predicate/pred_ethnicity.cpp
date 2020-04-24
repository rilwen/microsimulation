/*
(C) Averisera Ltd 2015
*/
#include "pred_ethnicity.hpp"
#include "../person.hpp"
#include "../contexts.hpp"
#include "core/preconditions.hpp"
#include <algorithm>

namespace averisera {
	namespace microsim {
		PredEthnicity::PredEthnicity(bool alive)
			: accept_dead_(!alive) {
		}

		bool PredEthnicity::life_signs_ok(const Person& obj, const Contexts& contexts) const {
			return accept_dead_ || obj.is_alive(contexts.asof());
		}

		bool PredEthnicity::select(const Person& obj, const Contexts& contexts) const {
			return life_signs_ok(obj, contexts) && select_out_of_context(obj);
		}

		void PredEthnicity::print(std::ostream& os) const {
			os << "Ethnicity" << get_name_suffix() << "(" << get_group_indices() << ", " << get_alive() << ")";
		}
	}
}
