#pragma once
#include "pred_ethnicity.hpp"
#include <unordered_set>

namespace averisera {
	namespace microsim {
		/*! Select Person object with this concrete ethnicity */
		class PredEthnicitySet : public PredEthnicity {
		public:
			/*! \throw std::domain_error If allowed.empty() */
			PredEthnicitySet(const std::unordered_set<ethnicity_type>& allowed, bool alive);

			/*! \throw std::domain_error If allowed.empty() */
			PredEthnicitySet(std::unordered_set<ethnicity_type>&& allowed, bool alive);

			PredEthnicitySet* clone() const override {
				return new PredEthnicitySet(allowed_, get_alive());
			}

			bool select_out_of_context(const Person& obj) const override;
		private:
			std::unordered_set<ethnicity_type> allowed_;

			std::vector<int> get_group_indices() const override;

			const char* get_name_suffix() const override {
				return "Set";
			}
		};
	}
}
