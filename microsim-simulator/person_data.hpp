#pragma once
#include "core/dates_fwd.hpp"
#include "microsim-core/person_attributes.hpp"
#include "actor_data.hpp"
#include "fetus.hpp"
#include <utility>
#include <vector>

namespace averisera {
    namespace microsim {

        /** Data for Person object, freely modifiable. Used when initialising Population or when sampling from it. */
        struct PersonData: public ActorData {            
            Actor::id_t mother_id; /** ID of this person's mother, if known (Actor::INVALID_ID otherwise) */
            std::vector<Actor::id_t> children; /**< Links to children which have their own data objects */
			std::vector<Date> childbirths; /**< Unlinked child births */
			std::vector<Fetus> fetuses; /**< Fetuses */
            Date date_of_birth; /**< Date of birth of this person */
            Date conception_date; /**< Conception date of this person if known (NAD otherwise) */
			Date date_of_death; /**< Date of death */
			Date immigration_date; /** Date of immigration */
            PersonAttributes attributes; /**< PersonAttributes */

            PersonData();
			PersonData(ActorData&& base);
            PersonData(const PersonData&) = delete;
            PersonData& operator=(const PersonData&) = delete;
            PersonData(PersonData&& other) noexcept;
            PersonData& operator=(PersonData&& other);
            void swap(PersonData& other) noexcept;
            
            PersonData clone() const;
			/** Clone and erase links with other Persons */
			PersonData clone_without_links() const;
			/** Clone and erase links with other Persons, setting a new ID */
			PersonData clone_without_links(Actor::id_t new_id) const;

            /** Reset loeaded IDs. Fix all links. */
            static void reset_ids(std::vector<PersonData>& data, MutableContext& mctx);

            /** Replace old child ID with new child ID
            @throw std::out_of_range If no child has ID == old_id */
            void change_child_id(Actor::id_t old_id, Actor::id_t new_id);

			/** Create a link to a child. 
			@throw std::domain_error If child.id is already link as child
			*/
            void link_child(PersonData& child, Date conception_date);

			/** Print to stream */
			void print(std::ostream& os) const;
        };

		inline void swap(PersonData& l, PersonData& r) {
			l.swap(r);
		}

		inline std::ostream& operator<<(std::ostream& os, const PersonData& data) {
			data.print(os);
			return os;
		}
    }
}
