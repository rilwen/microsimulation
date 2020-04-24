#include "migrant_selector_random.hpp"
#include "../contexts.hpp"
#include "../mutable_context.hpp"
#include "core/bootstrap.hpp"
#include "core/rng.hpp"

namespace averisera {
	namespace microsim {
		void MigrantSelectorRandom::select_impl(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& source, size_t number, std::vector<std::shared_ptr<Person>>::iterator dest_begin) const {
			RNG::StlWrapper stl_rng(ctx.mutable_ctx().rng());
			Bootstrap<RNG::StlWrapper> bootstrap(stl_rng);
			bootstrap.resample_without_replacement(source, dest_begin, dest_begin + number);
		}
	}
}
