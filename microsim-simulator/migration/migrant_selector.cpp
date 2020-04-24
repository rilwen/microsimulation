#include "migrant_selector.hpp"

namespace averisera {
	namespace microsim {
		MigrantSelector::~MigrantSelector() {}

		void MigrantSelector::select(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& source, std::vector<std::shared_ptr<Person>>& migrants, size_t number) const {
			const size_t old_removed_size = migrants.size();
			migrants.resize(old_removed_size + number);
			select_impl(ctx, source, number, migrants.begin() + old_removed_size);
		}
	}
}
