#pragma once
#include "pred_ethnicity.hpp"

namespace averisera {
	namespace microsim {
		/** Select Person object with this concrete ethnicity */
		class PredEthnicitySingle : public PredEthnicity {
		public:
			PredEthnicitySingle(ethnicity_type ethn, bool alive);

			PredEthnicitySingle* clone() const override {
				return new PredEthnicitySingle(ethn_, get_alive());
			}

			bool select_out_of_context(const Person& obj) const override;
		private:
			ethnicity_type ethn_;

			std::vector<int> get_group_indices() const override;

			const char* get_name_suffix() const override {
				return "Single";
			}
		};
	}
}
