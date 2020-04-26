// (C) Averisera Ltd 2014-2020
#include "pred_ethnicity_range.hpp"
#include "../person.hpp"

namespace averisera {
	namespace microsim {
		PredEthnicityRange::PredEthnicityRange(ethnicity_type from, ethnicity_type to, bool alive)
			: PredEthnicity(alive), from_(from), to_(to) {
			check_that(from <= to);
		}

		bool PredEthnicityRange::select_out_of_context(const Person& obj) const {
			const auto grp = obj.ethnicity();
			return grp >= from_ && grp <= to_;
		}

		std::vector<int> PredEthnicityRange::get_group_indices() const {
			std::vector<int> result(static_cast<size_t>(to_ - from_ + 1));
			std::iota(result.begin(), result.end(), static_cast<int>(from_));
			return result;
		}
	}
}
