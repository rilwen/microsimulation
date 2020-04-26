// (C) Averisera Ltd 2014-2020
#pragma once
#include "pred_ethnicity.hpp"

namespace averisera {
	namespace microsim {
		/** Select Person object with ethnicity in a [from, to] range */
		class PredEthnicityRange : public PredEthnicity {
		public:
			/** [from, to] inclusive range */
			PredEthnicityRange(ethnicity_type from, ethnicity_type to, bool alive);

			PredEthnicityRange* clone() const override {
				return new PredEthnicityRange(from_, to_, get_alive());
			}

			bool select_out_of_context(const Person& obj) const override;
		private:
			ethnicity_type from_;
			ethnicity_type to_;

			std::vector<int> get_group_indices() const override;

			const char* get_name_suffix() const override {
				return "Range";
			}
		};
	}
}
