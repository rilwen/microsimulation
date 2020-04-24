#include "migrant_selector_latest_immigrants_first.hpp"
#include "../person.hpp"
#include <algorithm>

namespace averisera {
	namespace microsim {
		void MigrantSelectorLatestImigrantsFirst::select_impl(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& source, size_t number, std::vector<std::shared_ptr<Person>>::iterator dest_begin) const {
			std::sort(source.begin(), source.end(), [](const std::shared_ptr<Person>& l, const std::shared_ptr<Person>& r) {
				assert(l);
				assert(r);
				if (r->immigration_date().is_not_a_date()) {
					return !l->immigration_date().is_not_a_date();
				} else {
					if (l->immigration_date().is_not_a_date()) {
						return r->immigration_date().is_not_a_date();
					} else {
						return l->immigration_date() > r->immigration_date();
					}
				}
			});
			std::copy(source.begin(), source.begin() + number, dest_begin);
		}
	}
}
