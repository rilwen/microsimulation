#include "actor_data.hpp"
#include "core/stl_utils.hpp"
#include <algorithm>

namespace averisera {
    namespace microsim {
        ActorData::ActorData()
            : id(Actor::INVALID_ID) {}

        ActorData::ActorData(ActorData&& other) noexcept
            : histories(std::move(other.histories)),
			id(other.id) {}

        ActorData& ActorData::operator=(ActorData&& other) {
            if (this != &other) {
                ActorData copy(std::move(other));
                this->swap(copy);
            }
            return *this;
        }

        void ActorData::swap(ActorData& other) noexcept {
            std::swap(id, other.id);
            histories.swap(other.histories);			
        }

        ActorData ActorData::clone() const {
            ActorData ad;
            ad.id = id;
			ad.histories = histories;
			return ad;
        }

		void ActorData::print(std::ostream& os) const {
			os << "ID=" << id << "\n";
			os << "HISTORIES=" << histories << "\n";
		}
    }
}
