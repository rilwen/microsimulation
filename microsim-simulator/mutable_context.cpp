/*
* (C) Averisera Ltd 2015
*/
#include "immutable_context.hpp"
#include "person.hpp"
#include "mutable_context.hpp"
#include <limits>
#include <stdexcept>
#include "core/rng_impl.hpp"

namespace averisera {
    namespace microsim {
		const std::string EMIGRANT_POPULATION_NAME("EMIGRANTS");

        MutableContext::MutableContext(long seed)
            : _rng(new RNGImpl(seed)), date_idx_(0), _max_id(0), emigrant_population_(EMIGRANT_POPULATION_NAME) {
        }

        MutableContext::MutableContext(std::unique_ptr<RNG>&& rngimpl)
            : _rng(std::move(rngimpl)), date_idx_(0), _max_id(0), emigrant_population_(EMIGRANT_POPULATION_NAME) {
            if (!_rng) {
                throw std::domain_error("MutableContext: null RNG");
            }
        }

        Actor::id_t MutableContext::gen_id() {
            if (_max_id == std::numeric_limits<Actor::id_t>::max()) {
                throw std::runtime_error("MutableContext: ran out of IDs");
            }
            ++_max_id;
            return _max_id;
        }

		void MutableContext::increase_id(Actor::id_t new_max_id) {
			if (new_max_id < get_max_id()) {
				throw std::domain_error("MutableContext: attempt to modify maximum ID to lower value");
			}
			_max_id = new_max_id;
		}

		void MutableContext::add_newborns(const std::vector<std::shared_ptr<Person>>& babies) {
			_newborns.reserve(_newborns.size() + babies.size());
			for (const auto& p : babies) {
				check_not_null(p, "MutableContext::add_newborns: null pointer");
				_newborns.push_back(p);
			}
			Population::sort_persons(_newborns);
		}

		void MutableContext::wipe_out_newborns() {
			_newborns.clear();
			_newborns.shrink_to_fit();
		}

		void MutableContext::add_emigrants(const std::vector<std::shared_ptr<Person>>& emigrants, Date emigration_date) {
			emigrants_.reserve(emigrants_.size() + emigrants.size());
			std::vector<std::shared_ptr<Person>>& for_date = emigrants_[emigration_date];
			Actor::id_t prev_id = 0;
			for (const auto& p : emigrants) {
				check_not_null(p, "MutableContext::add_emigrants: null pointer");
				const auto next_id = p->id();
				check_that(next_id > prev_id, "MutableContext::add_emigrants: not sorted by ID");
				for_date.push_back(p);
				prev_id = next_id;
			}
			Population::sort_persons(for_date);
			emigrant_population_.add_persons(emigrants, false);
		}

		void MutableContext::add_immigrants(const std::vector<std::shared_ptr<Person>>& immigrants) {
			immigrants_.reserve(immigrants_.size() + immigrants.size());
			Actor::id_t prev_id = 0;
			for (const auto& p : immigrants) {
				check_not_null(p, "MutableContext::add_immigrants: null pointer");
				const auto next_id = p->id();
				check_that(next_id > prev_id, "MutableContext::add_immigrants: not sorted by ID");
				immigrants_.push_back(p);
				prev_id = next_id;
			}
			Population::sort_persons(immigrants_);
		}

		Date MutableContext::asof(const ImmutableContext& imm_ctx) const {
			return imm_ctx.schedule().date(date_idx_);
		}
    }
}
