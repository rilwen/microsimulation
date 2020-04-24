/*
 ( *C) Averisera Ltd 2015
 */
#include "history_data.hpp"
#include "history_factory_registry.hpp"
#include "immutable_context.hpp"
#include "mutable_context.hpp"
#include "person.hpp"
#include "person_data.hpp"
#include "core/daycount.hpp"
#include "core/log.hpp"
#include "core/preconditions.hpp"
#include <cassert>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
		Person::Person(Actor::id_t id, PersonAttributes attribs, Date dob)
			: ActorImpl<Person>(id), _attribs(attribs), _dob(dob) {
            if (dob.is_not_a_date()) {
                throw std::domain_error("Person: date of birth invalid");
            }
			yob_ = dob.year();
            if (attribs.sex() == Sex::FEMALE) {
                _children = std::unique_ptr<child_vec_t>(new child_vec_t());
                _fetuses = std::unique_ptr<fetus_vec_t>(new fetus_vec_t());
            }
        }

		static void build_histories_from_data(PersonData::histories_t&& history_data, std::vector<std::unique_ptr<History>>& histories, const HistoryFactoryRegistry<Person>& registry, bool override_registry) {
			for (auto& hp : history_data) {
				if (registry.has_variable(hp.first)) {
					const auto idx = registry.variable_index(hp.first);
					if (!histories[idx]) { // histories[idx] != null means that we have a factory for that object
						if (override_registry) {
							histories[idx] = HistoryFactory::from_data(std::move(hp.second));
							LOG_WARN() << "Person: Overriding registry for variable " << hp.first;
						} else {
							LOG_WARN() << "Person: HistoryData present but no factory registered for variable " << hp.first;
							continue;
						}
					} else {
						HistoryFactory::append(*(histories[idx]), std::move(hp.second));						
					}
				} else {
					LOG_WARN() << "Person: HistoryData present but variable not registered: " << hp.first;
				}
			}
		}

        std::unique_ptr<Person> Person::from_data(PersonData&& data, const ImmutableContext& im_ctx, bool override_registry) {
            std::unique_ptr<Person> p(new Person(data.id, data.attributes, data.date_of_birth));
            const HistoryFactoryRegistry<Person>& registry = im_ctx.person_history_registry();
            std::vector<std::unique_ptr<History>> histories(registry.make_histories(*p));
			build_histories_from_data(std::move(data.histories), histories, registry, override_registry);			
			p->_conception_date = data.conception_date;
			p->set_histories(std::move(histories));
			for (Date cbd : data.childbirths) {
				p->add_childbirth(cbd);
			}
			if (!data.immigration_date.is_not_a_date()) {
				p->set_immigration_date(data.immigration_date);
			}
			if (!data.fetuses.empty()) {
				for (const auto& fetus : data.fetuses) {
					p->add_fetus(fetus);
				}
			}
            return p;
        }

        /*Person::Person(Actor::id_t id, PersonAttributes attribs, Date dob, std::weak_ptr<const Person> mother, Date conception_date)
            : Person(id, attribs, dob) {
            set_parents_data(mother, conception_date);
        }*/

        unsigned int Person::age(Date as_of) const {
			if (as_of < _dob) {
				return 0;
			}
			return static_cast<unsigned int>(_dob.dist_years(as_of));
        }
        
        double Person::age_fract(Date as_of) const {
			if (as_of < _dob) {
				return 0.;
			}
            return Daycount::YEAR_FRACT()->calc(_dob, as_of);
        }
        
        Person& Person::die(Date date) {
            _dod = date;
            return *this;
        }
        
        const HistoryFactoryRegistry<Person>& Person::get_history_registry(const ImmutableContext& im_ctx) const {
            return im_ctx.person_history_registry();
        }

        Person::child_idx_t Person::nbr_children() const {
            if (_children) {
                return static_cast<unsigned int>(_children->size());
            } else {
                return 0;
            }
        }

        std::shared_ptr<Person> Person::get_child(Person::child_idx_t idx) {
            if (idx < nbr_children()) {
                return (*_children)[idx].second;
            } else {
                throw std::out_of_range(boost::str(boost::format("Person: child born index %d too large (nbr of children born is %d") % idx % nbr_children()));
            }
        }

        std::shared_ptr<const Person> Person::get_child(Person::child_idx_t idx) const {
            if (idx < nbr_children()) {
                return (*_children)[idx].second;
            } else {
                throw std::out_of_range(boost::str(boost::format("Person: child born index %d too large (nbr of children born is %d") % idx % nbr_children()));
            }
        }

		void Person::sort_childbirths() {
			if (_children) {
				std::sort(_children->begin(), _children->end(), [](const child_t& a, const child_t& b) { return a.first < b.first; });
			}
		}

        void Person::add_child(std::shared_ptr<Person> child) {
			assert(child != nullptr);
             if (_children) {
                 for (auto c : *_children) {
                     if (c.second != nullptr && c.second->id() == child->id()) {
                         throw std::logic_error("Person: child being added twice");
                     }
                 }
				 _children->push_back(std::make_pair(child->date_of_birth(), child));				 
				 sort_childbirths();
             } else {
                 throw std::logic_error("Person: adding children unsupported for this object");
             }
        }

		void Person::add_childbirth(Date child_date_of_birth) {
			assert(!child_date_of_birth.is_special());
			if (_children) {
				_children->push_back(std::make_pair(child_date_of_birth, nullptr));
				sort_childbirths();
			} else {
				throw std::logic_error("Person: adding childbirths unsupported for this object");
			}
		}

        void Person::add_fetus(const Fetus& fetus) {
            if (_fetuses) {
                fetus_vec_t& fv = *_fetuses;
				check_that(!fetus.conception_date().is_not_a_date(), "Person::add_fetus: conception date cannot be NAD");
                if (fv.empty() || fetus.conception_date() >= fv.back().conception_date()) {
                    fv.push_back(fetus);
                } else {
					LOG_ERROR() << "Person::add_fetus: adding fetus with CD=" << fetus.conception_date() << " before fetus with CD=" << fv.back().conception_date();
                    throw std::logic_error("Person::add_fetus: cannot add this Fetus");
                }
            } else {
                throw std::logic_error("Person::add_fetus: adding fetuses unsupported");
            }
        }

        void Person::remove_fetuses(const Date date) {
            // also called from give_birth()
            if (_fetuses && !_fetuses->empty()) {
				// handle the most common case
				const Date d1 = _fetuses->front().conception_date();
				if (_fetuses->back().conception_date() == d1) {
					if (d1 < date) {
						// remove all fetuses
                        std::vector<Fetus> empty;
						(*_fetuses).swap(empty);
					}
				} else {
					std::vector<Fetus> remaining;
					std::copy_if(_fetuses->begin(), _fetuses->end(), std::back_inserter(remaining), [date](const Fetus& fetus) { return fetus.conception_date() >= date; });
					remaining.shrink_to_fit();
					remaining.swap(*_fetuses);
				}
            }
        }

		Person::child_idx_t Person::nbr_fetuses() const {
			if (_fetuses) {
				return static_cast<child_idx_t>(_fetuses->size());
			} else {
				return 0;
			}
		}

        void Person::give_birth(const Date date, const Contexts& ctx, Person::shared_ptr this_mother) {
            if (_fetuses && !_fetuses->empty()) {
                assert(_children);
                if (!_children->empty() && date < _children->back().first) {
					throw std::logic_error("Person: future children already stored");
				}
				MutableContext& mc = ctx.mutable_ctx();
				const HistoryFactoryRegistry<Person>& registry = ctx.immutable_ctx().person_history_registry();
                for (Fetus fetus: *_fetuses) {
					if (fetus.conception_date() < date) {
						const Person::shared_ptr child = Person::make_shared(mc.gen_id(), fetus.attributes(), date);
						std::vector<std::unique_ptr<History>> histories(registry.make_histories(*child));
						child->set_histories(std::move(histories));
						LOG_TRACE() << "Person: giving birth to child with ID " << child->id() << " on " << date;
						Person::link_parents_child(child, this_mother, fetus.conception_date());
					}
                }
                remove_fetuses(date);
            }
        }

        /*std::unique_ptr<Person> Person::partial_clone(Actor::id_t new_id) const {
            std::unique_ptr<Person> cloned(new Person(new_id, _attribs, _dob));
            if (this->died()) {
                cloned->die(_dod);
            }
            const Actor::histidx_t nhist = this->nbr_histories();
            std::vector<std::unique_ptr<History>> cloned_histories;
            cloned_histories.reserve(nhist);
            for (Actor::histidx_t i = 0; i < nhist; ++i) {
                if (this->is_history_valid(i)) {
                    cloned_histories.push_back(this->history(i).clone());
                } else {
                    cloned_histories.push_back(nullptr);
                }
            }
            cloned->set_histories(std::move(cloned_histories));
            for (const child_t& chld : *_children) {
                if (!chld.second) {
                    cloned->add_childbirth(chld.first);
                }
            }
            return std::move(cloned);
        }*/

		bool Person::is_parent_of(const Person& other) const {
			if (_children) {
				// TODO: optimize
				for (auto& c : *_children) {
					if (c.second && c.second.get() == &other) {
						return true;
					}
				}
			}
			return false;
		}

        void Person::set_parents_data(std::weak_ptr<const Person> mother, Date conception_date) {
            _mother = mother;
            _conception_date = conception_date;
        }
        
        void Person::link_parents_child(std::shared_ptr<Person> child, std::shared_ptr<Person> mother, Date conception_date) {
            if (!child) {
                throw std::domain_error("Person: child is null");
            }
            if (!mother) {
                throw std::domain_error("Person: mother is null");
            }
			check_that(child != mother, "Person: mother equals to child");
            if (mother->sex() != Sex::FEMALE) {
                throw std::domain_error("Person: mother is not female");
            }
            if (conception_date.is_not_a_date()) {
                throw std::domain_error("Person: conception date invalid");
            }
			if (conception_date <= mother->date_of_birth()) {
				throw std::domain_error("Person: conception date on or before mother's date of birth");
			}
			if (conception_date >= child->date_of_birth()) {
				throw std::domain_error("Person: conception date on or after child's date of birth");
			}
            if (!child->_conception_date.is_not_a_date()) {
                std::logic_error("Person: child parents data already set");
            }
            child->set_parents_data(std::weak_ptr<Person>(mother), conception_date);
            mother->add_child(child);
        }

        void Person::unlink_children(Date since) {
            if (_children) {
                for (auto& date_ptr: *_children) {
                    if (date_ptr.first >= since) {
                        date_ptr.second = nullptr;
                    }
                }
            }
        }

		PersonData Person::to_data(const ImmutableContext& im_ctx) const {
			PersonData pd(Actor::to_data(im_ctx));
			pd.date_of_birth = _dob;
			pd.conception_date = _conception_date;
			pd.attributes = _attribs;
			pd.date_of_death = _dod;
			if (_children) {
				for (const auto& cp : *_children) {
					if (cp.second) {
						pd.children.push_back(cp.second->id());
					} else {
						pd.childbirths.push_back(cp.first);
					}
				}
			}
			pd.immigration_date = immigration_date_;
			if (_fetuses) {
				pd.fetuses = *_fetuses;
			}
			if (const auto mother_ptr = _mother.lock()) {
				pd.mother_id = mother_ptr->id();
			} else {
				pd.mother_id = Actor::INVALID_ID;
			}
			return pd;
		}

		void Person::set_immigration_date(Date imdate) {
			if (imdate.is_special()) {
				throw std::domain_error("Person: ordinary immigration date expected");
			}
			if (imdate >= _dob) {
				if (_dod.is_not_a_date() || imdate <= _dod) {
					immigration_date_ = imdate;
				} else {
					LOG_ERROR() << "Person: immigration date " << imdate << " after date of death " << _dod;
					throw std::domain_error("Person: immigration date must be on or before date of death");
				}
			} else {
				LOG_ERROR() << "Person: immigration date " << imdate << " before date of birth " << _dob;
				throw std::domain_error("Person: immigration date must be on or after date of birth");
			}
		}
    }
}
