#pragma once
#include "migrant_selector.hpp"

namespace averisera {
	namespace microsim {
		/*! Selects migrants randomly */
		class MigrantSelectorRandom : public MigrantSelector {
		private:
			void select_impl(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& source, size_t number, std::vector<std::shared_ptr<Person>>::iterator dest_begin) const override;
		};
	}
}
