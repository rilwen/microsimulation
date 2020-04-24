#include "pred_ethnicity_single.hpp"
#include "../person.hpp"

namespace averisera {
	namespace microsim {
		PredEthnicitySingle::PredEthnicitySingle(ethnicity_type ethn, bool alive)
			: PredEthnicity(alive), ethn_(ethn) {}

		bool PredEthnicitySingle::select_out_of_context(const Person& obj) const {
			return obj.ethnicity() == ethn_;
		}

		std::vector<int> PredEthnicitySingle::get_group_indices() const {
			return std::vector<int>({ static_cast<int>(ethn_) });
		}
	}
}
