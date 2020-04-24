#include "contexts.hpp"
#include "immutable_context.hpp"
#include "person.hpp"
#include "person_data.hpp"
#include "population.hpp"
#include "population_data.hpp"
#include "predicate.hpp"
#include <algorithm>
#include <cassert>
#include <iterator>
#include <stdexcept>
#include "core/log.hpp"
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
		Population::Population(const std::string& name)
			: name_(name) {}

        Population::Population(Population&& other)
            : name_(std::move(other.name_)), _persons(std::move(other._persons)) {
            other._persons.resize(0);
        }
        
        void Population::add_person(Person::shared_ptr person, bool check_id) {
            if (!person) {
                throw std::domain_error("Population: null person");
            }
			if ((!check_id) || _persons.empty() || _persons.back()->id() < person->id()) {
				LOG_TRACE() << "Population " << name_ << ": added Person with ID " << person->id();
				_persons.push_back(person);
			} else {
				LOG_ERROR() << "Population " << name_ << ": error adding Person(DOB=" << person->date_of_birth() << ", SEX=" << person->sex() << ", ETHN=" << int(person->ethnicity()) << ", ID=" << person->id() << "): max ID=" << _persons.back()->id();
				const Person& p = *(_persons.back());
				LOG_ERROR() << "Population " << name_ << ": existing Person with this ID is DOB=" << p.date_of_birth() << ", SEX=" << p.sex() << ", ETHN=" << int(p.ethnicity());
				throw std::domain_error("Population: IDs not increasing");
			}			
        }

        void Population::add_persons(const std::vector<Person::shared_ptr>& new_persons, bool check_ids) {
			LOG_TRACE() << "Population: adding " << new_persons.size() << " new Persons";
            _persons.reserve(_persons.size() + new_persons.size());
			Actor::id_t prev_id = 0;
            for (const auto& p: new_persons) {
				if (!p) {
					throw std::domain_error("Population::add_persons: null Person pointer");
				}
				const auto next_id = p->id();
				check_that(next_id > prev_id, "Population::add_persons: added persons not sorted by ID");
                add_person(p, check_ids);
				prev_id = next_id;
            }
        }

        void Population::import_persons(std::vector<PersonData>& person_datas, const Contexts& ctx, const bool keep_ids, const bool immigration) {
			if (person_datas.empty()) {
				return;
			}
            if (!keep_ids) {
				LOG_TRACE() << "Population " << name_ << ": resetting Person IDs";
                PersonData::reset_ids(person_datas, ctx.mutable_ctx());
            }
            ActorData::sort_by_id(person_datas);			
			LOG_DEBUG() << "Population " << name_ << ": adding PersonData vec with min. ID " << person_datas.front().id << " and max. ID " << person_datas.back().id;
            std::vector<Person::shared_ptr> added_persons;
            added_persons.reserve(person_datas.size());
            const ImmutableContext& im_ctx = ctx.immutable_ctx();
            for (PersonData& pd: person_datas) {
                added_persons.push_back(Person::from_data(std::move(pd), im_ctx));
            }
			if (immigration) {
				ctx.mutable_ctx().add_immigrants(added_persons);
			}
            link_parents_children(added_persons, person_datas);
            add_persons(added_persons);
        }

        void Population::import_data(PopulationData& data, const Contexts& ctx, bool keep_ids, bool immigration) {
            import_persons(data.persons, ctx, keep_ids, immigration);
        }
        
        Person::shared_ptr Population::get_person(Actor::id_t id) const {
            const auto it = std::lower_bound(_persons.begin(), _persons.end(), id, 
                                             [](const Person::shared_ptr& p, unsigned long id) { return p->id() < id; });
            if (it == _persons.end() || (*it)->id() != id) {
                return nullptr;
            } else {
                return *it;
            }
        }

        void Population::merge(const Population& other) {
            merge_persons(_persons, other._persons);
        }        

        void Population::wipe_out() {
            std::vector<Person::shared_ptr>().swap(_persons); // force freeing memory
        }

        void Population::transfer_persons(Population& source, const Predicate<Person>& selector, const Contexts& ctx) {
            if (!source._persons.empty()) {				
                std::vector<Person::shared_ptr> elect;
                std::vector<Person::shared_ptr> preterite; // cf. Gravity's Rainbow
                const auto end = source._persons.end();
                for (auto it = source._persons.begin(); it != end; ++it) {
                    if (selector.select(**it, ctx)) {
                        elect.push_back(*it);
                    } else {
                        preterite.push_back(*it);
                    }
                }
				// elect and preterite are sorted by ID because source._persons is
				elect.shrink_to_fit();
				preterite.shrink_to_fit();
				LOG_TRACE() << "Population " << name_ << ": transferring " << elect.size() << " Persons to Population and leaving " << preterite.size() << " behind";
                if (!_persons.empty()) {
                    merge_persons(_persons, elect);
                } else {
                    _persons.swap(elect);
                }
                source._persons.swap(preterite);
            }
        }

        void Population::merge_persons(std::vector<Person::shared_ptr>& dst, const std::vector<Person::shared_ptr>& src) {
            if (!src.empty()) {
                if (!dst.empty()) {
                    std::vector<Person::shared_ptr> newdst(dst.size() + src.size());
                    std::merge(dst.begin(), dst.end(), src.begin(), src.end(), newdst.begin(), Person::compare_ptr_by_id);
                    assert(newdst.size() >= 2);
                    auto prev_id = newdst.front()->id();
                    for (auto it = newdst.begin() + 1; it != newdst.end(); ++it) {
                        const auto next_id = (*it)->id();
                        assert(next_id >= prev_id);                        
                        if (prev_id == next_id) {
                            if (*it == *(it - 1)) {
                                throw std::logic_error("Population: the same person in both merged populations");
                            } else {
                                throw std::logic_error("Population: merged in persons with duplicate IDs");
                            }
                        }
                        prev_id = next_id;
                    }
                    dst.swap(newdst);
                } else {
                    dst = src;
                }
            }
        }

        void Population::link_parents_children(std::vector<Person::shared_ptr>& persons, const std::vector<PersonData>& person_data) {
            if (persons.size() != person_data.size()) {
                throw std::domain_error("Person: persons and data vectors have different sizes");
            }
            for (const PersonData& pd : person_data) {
                if (pd.mother_id != Actor::INVALID_ID) {
                    const Person::shared_ptr child = find_by_id<Person>(persons, pd.id);
                    if (!child) {
                        throw std::domain_error(boost::str(boost::format("Population: child ID %d has no Person") % pd.id));
                    }
                    const Person::shared_ptr mother = find_by_id<Person>(persons, pd.mother_id);
                    if (!mother) {
                        throw std::domain_error(boost::str(boost::format("Population: mother ID %d has no Person") % pd.mother_id));
                    }
                    Person::link_parents_child(child, mother, pd.conception_date);
                }
            }
        }

		bool Population::empty() const {
			return _persons.empty();
		}

		void Population::remove_persons(const std::vector<Actor::shared_ptr<Person>>& removed_persons) {
			if (removed_persons.empty()) {
				return;
			}
			if (_persons.empty()) {
				throw std::domain_error("Population: removing persons from empty population");
			}
			std::vector<Actor::shared_ptr<Person>> copy(_persons);
			auto copy_it = copy.begin();
			// mark removed as null
			Actor::id_t prev_id = Actor::MIN_ID - 1;
			for (auto removed_it = removed_persons.begin(); removed_it != removed_persons.end(); ++removed_it) {
				assert(copy_it != copy.end());
				assert(*copy_it);
				if (*removed_it == nullptr) {
					throw std::domain_error("Population: removing null Person");
				}
				const auto removed_id = (*removed_it)->id();
				check_not_equals(removed_id, Actor::INVALID_ID, "Population::remove_persons: removed person has invalid ID");
				if (removed_id <= prev_id) {
					LOG_ERROR() << "Population::remove_persons: removed persons not sorted by ID: " << prev_id << " followed by " << removed_id;
					throw std::domain_error("Population::remove_persons: removed persons not sorted by ID");
				}
				while (copy_it != copy.end() && (*copy_it)->id() < removed_id) {
					++copy_it;
				}
				if (copy_it != copy.end()) {
					*copy_it = nullptr;
					++copy_it;
				} else {
					throw std::domain_error("Population: removing a Person not present in population - possible duplicate in removed_persons");
				}
				prev_id = removed_id;
			}
			// remove the marked ones
			const auto to_erase_it = std::remove(copy.begin(), copy.end(), nullptr);
			copy.erase(to_erase_it, copy.end());
			copy.shrink_to_fit();
			assert(removed_persons.size() == _persons.size() - copy.size());
			LOG_DEBUG() << "Population " << name_ << ": removed " << (_persons.size() - copy.size()) << " persons";
			// persist new population
			_persons.swap(copy);
		}		

		std::vector<Actor::shared_ptr<Person>> Population::live_persons(const Date asof) const {
			std::vector<std::shared_ptr<Person>> live_persons;
			live_persons.reserve(_persons.size());
			for (const auto& person_ptr : _persons) {
				if (person_ptr->is_alive(asof)) {
					live_persons.push_back(person_ptr);
				}
			}
			return live_persons;
		}

		void Population::sort_persons(std::vector<Actor::shared_ptr<Person>>& persons) {
			std::sort(persons.begin(), persons.end(), Person::compare_ptr_by_id);
		}
    }
}
