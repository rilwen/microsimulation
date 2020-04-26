// (C) Averisera Ltd 2014-2020
#pragma once
#include "migrant_selector.hpp"

namespace averisera {
	namespace microsim {
		/** Selects latest imigrants first. Use for FIFO emigration model. */
		class MigrantSelectorLatestImigrantsFirst : public MigrantSelector {
		private:
			void select_impl(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& source, size_t number, std::vector<std::shared_ptr<Person>>::iterator dest_begin) const override;
		};
	}
}
