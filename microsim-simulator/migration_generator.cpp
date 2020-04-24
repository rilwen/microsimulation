#include "migration_generator.hpp"
#include "mutable_context.hpp"
#include "person.hpp"
#include "person_data.hpp"
#include "microsim-core/schedule_period.hpp"
#include "core/dates.hpp"
#include "core/period.hpp"
#include "core/stl_utils.hpp"

namespace averisera {
	namespace microsim {
		MigrationGenerator::~MigrationGenerator() {
		}

		Date MigrationGenerator::calc_migration_date(const SchedulePeriod& sp) {
			const Date md = sp.begin + (sp.end - sp.begin) / 2;
			assert(md >= sp.begin);
			assert(md <= sp.end);
			return md;
		}

		std::vector<std::shared_ptr<Person>> MigrationGenerator::comigrate_children(std::vector<std::shared_ptr<Person>>::iterator begin, std::vector<std::shared_ptr<Person>>::iterator end, Date migration_date, unsigned int comigrated_child_age_limit) {
			std::vector<std::shared_ptr<Person>> comigrated_children;
			for (auto it = begin; it != end; ++it) {
				assert(*it);
				Person& rp = **it;
				const auto nbrchld = rp.nbr_children();
				for (Person::child_idx_t cidx = 0; cidx < nbrchld; ++cidx) {
					const auto& cptr = rp.get_child(cidx);
					if (cptr && (cptr->date_of_birth() >= migration_date || cptr->age(migration_date) < comigrated_child_age_limit)) {
						comigrated_children.push_back(cptr);
					}
				}
			}
			return comigrated_children;
		}

		size_t MigrationGenerator::comigrate_children(std::vector<std::shared_ptr<Person>>& adults, Date migration_date, unsigned int comigrated_child_age_limit, size_t parents_begin_idx) {
			std::vector<std::shared_ptr<Person>> comigrated_children(comigrate_children(adults.begin() + parents_begin_idx, adults.end(), migration_date, comigrated_child_age_limit));
			adults.insert(adults.end(), comigrated_children.begin(), comigrated_children.end());
			return comigrated_children.size();
		}

		void MigrationGenerator::convert_added_persons_to_data(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& persons_added_ptrs, const unsigned int comigrated_child_age_limit, std::vector<PersonData>& persons_added_data, Date immigration_date) {
			// clone the added persons			
			Population::sort_persons(persons_added_ptrs);
			const size_t old_nbr_persons_added = persons_added_data.size();
			persons_added_data.reserve(old_nbr_persons_added + persons_added_ptrs.size());
#ifndef NDEBUG
			const Date asof = ctx.asof();
#endif // !NDEBUG
			for (const auto& ptr : persons_added_ptrs) {
				persons_added_data.push_back(ptr->to_data(ctx.immutable_ctx()));
				persons_added_data.back().immigration_date = std::max(immigration_date, persons_added_data.back().date_of_birth);
			}
			// change IDs
			std::unordered_map<Actor::id_t, std::list<Actor::id_t>> old_to_new; // map old IDs to new IDs
			for (auto it = persons_added_data.begin() + old_nbr_persons_added; it != persons_added_data.end(); ++it) {
				const auto old_id = it->id;
				const auto new_id = ctx.mutable_ctx().gen_id(); // generate a fresh new ID
				if (old_to_new.find(old_id) == old_to_new.end()) {
					old_to_new.insert(std::make_pair(old_id, std::list<Actor::id_t>({ new_id })));
				} else {
					old_to_new[old_id].push_back(new_id);
				}
				it->id = new_id; // assign added person a new ID
			}
			// change child and mother's IDs to new values
			auto ptr_it = persons_added_ptrs.begin();
			for (auto data_it = persons_added_data.begin() + old_nbr_persons_added; data_it != persons_added_data.end(); ++data_it, ++ptr_it) {
				assert(ptr_it != persons_added_ptrs.end());
				Person& person = **ptr_it;
				const auto& new_ids = old_to_new[person.id()];
				const auto new_id_iter = std::find(new_ids.begin(), new_ids.end(), data_it->id);
				assert(new_id_iter != new_ids.end());
				const size_t copy_idx = static_cast<size_t>(std::distance(new_ids.begin(), new_id_iter)); // for old IDs used multiple times, copy_idx tells is which copy we are using
				Actor::id_t new_mother_id = Actor::INVALID_ID;
				const auto new_mother_ids_iter = old_to_new.find(data_it->mother_id);				
				if (new_mother_ids_iter != old_to_new.end()) { // find which new mother this copy has
					if (new_mother_ids_iter->second.size() > copy_idx) {
						new_mother_id = *std::next(new_mother_ids_iter->second.begin(), copy_idx);
						assert(new_mother_id != Actor::INVALID_ID);
					} else {
						LOG_WARN() << "MigrationGenerator::convert_added_persons_to_data: cannot find new mother's ID for Person added on " << immigration_date << ": " << *data_it;
					}
				} else {
					if (!data_it->conception_date.is_not_a_date() && person.age(immigration_date) < comigrated_child_age_limit) {
						LOG_WARN() << "MigrationGenerator::convert_added_persons_to_data: cannot find mother's ID for Person added on " << immigration_date << ": " << *data_it;
					}					
				}
				assert(new_mother_id != Actor::INVALID_ID || (person.date_of_birth() < asof && person.age(asof) >= comigrated_child_age_limit));
				data_it->mother_id = new_mother_id; // point to "new mother"
				size_t child_id_idx = 0;
				const auto nbr_children = person.nbr_children();
				for (Person::child_idx_t cidx = 0; cidx < nbr_children; ++cidx) {
					auto child_ptr = person.get_child(cidx);
					if (child_ptr) {
						assert(child_ptr->id() == data_it->children[child_id_idx]);
						// new child ID
						const auto new_child_ids_iter = old_to_new.find(child_ptr->id());
						Actor::id_t new_id = Actor::INVALID_ID;
						if (new_child_ids_iter != old_to_new.end()) {
							if (new_child_ids_iter->second.size() > copy_idx) {
								new_id = *std::next(new_child_ids_iter->second.begin(), copy_idx);
							}
						}
						data_it->children[child_id_idx] = new_id; // if new_id == Actor::INVALID_ID, it will be removed later
						if (new_id == Actor::INVALID_ID) {
							data_it->childbirths.push_back(child_ptr->date_of_birth());
						}
						++child_id_idx;
					} // else the date is already in data_it->childbirths							
				}
				assert(child_id_idx == data_it->children.size());
				std::sort(data_it->childbirths.begin(), data_it->childbirths.end());
				assert(data_it->childbirths.size() <= nbr_children);
				const size_t nbr_ids_to_keep = nbr_children - data_it->childbirths.size();
				std::vector<Actor::id_t> new_child_ids(nbr_ids_to_keep, Actor::INVALID_ID);
				// keep only valid new child IDs
				const static auto copy_pred = [](Actor::id_t id) { return id != Actor::INVALID_ID; };
				std::copy_if(data_it->children.begin(), data_it->children.end(), new_child_ids.begin(), copy_pred);
				assert(std::all_of(new_child_ids.begin(), new_child_ids.end(), copy_pred));
				data_it->children = std::move(new_child_ids);
			}
			assert(ptr_it == persons_added_ptrs.end());
		}

		size_t MigrationGenerator::remove_below_age(const Date asof, const unsigned int min_age, std::vector<std::shared_ptr<Person>>& selected) {
			const size_t old_size = selected.size();
			selected.erase(std::remove_if(selected.begin(), selected.end(), [asof, min_age](const std::shared_ptr<Person>& p) {
				return !(p->is_alive(asof) && static_cast<unsigned int>(p->age(asof)) >= min_age);
			}), selected.end());
			return old_size - selected.size();
		}
	}
}
