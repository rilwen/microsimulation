#include "person_data.hpp"
#include "core/stl_utils.hpp"
#include <algorithm>
#include <cassert>

namespace averisera {
    namespace microsim {
        PersonData::PersonData()
            : mother_id(Actor::INVALID_ID) {}

		PersonData::PersonData(ActorData&& base)
			: ActorData(std::move(base)), mother_id(Actor::INVALID_ID) {
		}

        PersonData::PersonData(PersonData&& other) noexcept
            : ActorData(std::move(other)),
            mother_id(other.mother_id),
            children(std::move(other.children)),
			childbirths(std::move(other.childbirths)),
			fetuses(std::move(other.fetuses)),
            date_of_birth(other.date_of_birth),
            conception_date(other.conception_date),
            date_of_death(other.date_of_death),
			immigration_date(other.immigration_date),
			attributes(other.attributes)
		{}

        PersonData& PersonData::operator=(PersonData&& other) {
            if (this != &other) {
                PersonData copy(std::move(other));
                this->swap(copy);
            }
            return *this;
        }

        void PersonData::swap(PersonData& other) noexcept {
            ActorData::swap(other);
            std::swap(mother_id, other.mother_id);
            std::swap(children, other.children);
			std::swap(childbirths, other.childbirths);
			std::swap(fetuses, other.fetuses);
            std::swap(date_of_birth, other.date_of_birth);
            std::swap(conception_date, other.conception_date);
            std::swap(date_of_death, other.date_of_death);
			std::swap(immigration_date, other.immigration_date);
			std::swap(attributes, other.attributes);
        }

        PersonData PersonData::clone() const {
            PersonData copy(ActorData::clone());
            copy.mother_id = mother_id;
            copy.children = children;
			copy.childbirths = childbirths;
			copy.fetuses = fetuses;
            copy.date_of_birth = date_of_birth;
            copy.conception_date = conception_date;
            copy.date_of_death = date_of_death;
			copy.immigration_date = immigration_date;
			copy.attributes = attributes;
			return copy;
        }

		PersonData PersonData::clone_without_links() const {
			PersonData copy(this->clone());
			copy.mother_id = Actor::INVALID_ID;
			copy.children.clear();
			copy.children.shrink_to_fit();
			return copy;
		}

		PersonData PersonData::clone_without_links(Actor::id_t new_id) const {
			PersonData copy(this->clone_without_links());
			copy.id = new_id;
			return copy;
		}

        void PersonData::reset_ids(std::vector<PersonData>& data, MutableContext& mctx) {
            ActorData::id_map_t old_to_new;
            //id_map_t new_to_old;
            ActorData::reset_ids(data, mctx, old_to_new/*, new_to_old*/);
            for (PersonData& pd : data) {
                if (pd.mother_id != Actor::INVALID_ID) {
                    assert(StlUtils::contains(old_to_new, pd.mother_id));
                    pd.mother_id = old_to_new[pd.mother_id];
                }
                for (id_t& cd : pd.children) {
                    if (cd != Actor::INVALID_ID) {
                        assert(StlUtils::contains(old_to_new, cd));
                        cd = old_to_new[cd];
                    }
                }
            }
        }

        void PersonData::change_child_id(const Actor::id_t old_id, const Actor::id_t new_id) {
            for (Actor::id_t& c_id : children) {
                if (c_id == old_id) {
                    c_id = new_id;
                    return;
                }
            }
            throw std::out_of_range("PersonData: no such child ID");
        }

        void PersonData::link_child(PersonData& child, Date conception_date) {
			if (std::find(children.begin(), children.end(), child.id) != children.end()) {
				throw std::domain_error("PersonData::link_child: ID already linked");
			}
			children.push_back(child.id); // do this first in case insert fails
            child.mother_id = id;			
			child.conception_date = conception_date;
        }

		void PersonData::print(std::ostream& os) const {
			ActorData::print(os);
			os << "ATTRIBS=" << attributes << "\n";
			os << "DOB=" << date_of_birth << "\n";
			os << "DOD=" << date_of_death << "\n";
			os << "CONCEPTION_DATE=" << conception_date << "\n";
			os << "IMMIGRATION_DATE=" << immigration_date << "\n";
			os << "MOTHER_ID=" << mother_id << "\n";
			os << "CHILDREN=" << children << "\n";
			os << "CHILDBIRTHS=" << childbirths << "\n";
			os << "FETUSES=" << fetuses << "\n";
		}
    }
}
