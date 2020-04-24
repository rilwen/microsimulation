#include "actor.hpp"
#include "actor_data.hpp"
#include "contexts.hpp"
#include "history_registry.hpp"
#include <cassert>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		Actor::Actor(id_t id)
			: _id(id)
		{
			validate();
		}

		Actor::~Actor() {
		}

        void Actor::set_histories(std::vector<std::unique_ptr<History>>&& histories) {
            if (!_histories.empty()) {
				throw std::logic_error("Actor: histories already set");
            }
            _histories = std::move(histories);
        }

        bool Actor::is_history_valid(histidx_t idx) const {
            return (idx < nbr_histories() && _histories[idx]);
        }

		History& Actor::history(histidx_t idx) {
            if (is_history_valid(idx)) {
                return *_histories[idx];
            } else {
				throw std::domain_error(boost::str(boost::format("Actor: no valid history with index %d for ID %d") % idx % _id));
            }
		}

		const History& Actor::history(histidx_t idx) const {
			if (is_history_valid(idx)) {
                return *_histories[idx];
            } else {
                throw std::domain_error(boost::str(boost::format("Actor: no valid history with index %d for ID %d (const)") % idx % _id));
            }
		}

        bool Actor::has_history(const ImmutableContext& im_ctx, const std::string& variable) const {
            const auto& registry = get_history_registry(im_ctx);
            if (registry.has_variable(variable)) {
                return is_history_valid(registry.variable_index(variable));
            } else {
                return false;
            }
        }

        History& Actor::history(const ImmutableContext& im_ctx, const std::string& variable) {
            const auto& registry = get_history_registry(im_ctx);
            const histidx_t idx = registry.variable_index(variable);
            return history(idx);
        }

        const History& Actor::history(const ImmutableContext& im_ctx, const std::string& variable) const {
            const auto& registry = get_history_registry(im_ctx);
            const histidx_t idx = registry.variable_index(variable);
            return history(idx);
        }

        Actor::histidx_t Actor::get_variable_index(const Contexts& ctx, const std::string& variable) const {
            return get_history_registry(ctx.immutable_ctx()).variable_index(variable);
        }

		void Actor::validate() const {
            // Turn off null checks -- we can have null histories if we do not plan to use the history on given object
			// for (auto it = _histories.begin(); it != _histories.end(); ++it) {
			// 	if (*it == nullptr) {
			// 		throw std::domain_error("Person: Null history object");
			// 	}
			// }
            if (!_id) {
                throw std::domain_error("Actor: ID must be non-zero");
            }
		}

		ActorData Actor::to_data(const ImmutableContext& im_ctx) const {
			ActorData ad;
			ad.id = _id;
			const size_t nh = _histories.size();
			const HistoryRegistry& hreg = get_history_registry(im_ctx);
			for (size_t i = 0; i < nh; ++i) {
				if (_histories[i]) {
					ad.histories[hreg.variable_name(i)] = _histories[i]->to_data();
				}
			}
			return ad;
		}

        // so that the library binary contains the constant
        const Actor::id_t Actor::INVALID_ID;
        const Actor::id_t Actor::MIN_ID;
	}
}
