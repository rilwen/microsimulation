#include "pred_ethnicity_set.hpp"
#include "../person.hpp"

namespace averisera {
	namespace microsim {
		PredEthnicitySet::PredEthnicitySet(const std::unordered_set<ethnicity_type>& allowed, bool alive)
			: PredEthnicity(alive), allowed_(allowed) {
			check_that(!allowed_.empty(), "PredEthnicitySet: allowed set cannot be empty");
		}

		PredEthnicitySet::PredEthnicitySet(std::unordered_set<ethnicity_type>&& allowed, bool alive)
			: PredEthnicity(alive), allowed_(std::move(allowed)) {
			check_that(!allowed_.empty(), "PredEthnicitySet: allowed set cannot be empty");
		}

		bool PredEthnicitySet::select_out_of_context(const Person& obj) const {
			return allowed_.find(obj.ethnicity()) != allowed_.end();
		}

		std::vector<int> PredEthnicitySet::get_group_indices() const {
			std::vector<int> result(allowed_.begin(), allowed_.end());
			std::sort(result.begin(), result.end());
			return result;
		}
	}
}
