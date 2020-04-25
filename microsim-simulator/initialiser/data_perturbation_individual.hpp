#pragma once
#include "data_perturbation.hpp"

namespace averisera {
	namespace microsim {
		/** DataPerturbation which acts on individual data objects */
		template <class AD> class DataPerturbationIndividual: public DataPerturbation<AD> {
		public:
			void apply(std::vector<AD>& datas, const Contexts& ctx) const override {
				for (AD& data : datas) {
					apply(data, ctx);
				}
			}
		private:
			virtual void apply(AD& data, const Contexts& ctx) const = 0;
		};
	}
}