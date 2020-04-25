/*
 (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_PERSON_H
#define __AVERISERA_MS_PERSON_H

#include "actor.hpp"
#include "fetus.hpp"
#include "history_factory_registry.hpp"
#include "core/dates.hpp"
#include "microsim-core/person_attributes.hpp"
#include "history.hpp"
#include <memory>
#include <utility>
#include <vector>

namespace averisera {
    namespace microsim {
        template <class T> class HistoryFactoryRegistry;
        class MutableContext;
        struct PersonData;
        
        /**
         *  @brief Represents a person in a simulated population.
         */
        class Person: public ActorImpl<Person> {
        public:
            /**
             * @param[in] id ID number
			 * @param[in] histories Moveable vector of History objects
             * @param[in] attribs Fixed attributes
             * @param[in] dob Date of birth			 
             @throw std::domain_error If dob is not-a-date.
             */
			Person(Actor::id_t id, PersonAttributes attribs, Date dob);

            /** Construct a Person from PersonData POD object. If data has histories which are not registered, they will be ignored. Others will be moved to the created object (to save memory).            
            @param override_registry If true, insert histories from data even if the created Person object is not assigned a history factory in the context.
            @throw std::domain_error If data are invalid
            */
            static std::unique_ptr<Person> from_data(PersonData&& data, const ImmutableContext& im_ctx, bool override_registry = false);

            ///**
            // * @param[in] id ID number
            // * @param[in] attribs Fixed attributes
            // * @param[in] dob Date of birth			 
            // * @param[in] mother Person's mother
            // * @param[in] conception_date Date the person was conceived
            // @throw std::domain_error If dob is not-a-date, mother is null or conception_date is Not-a-date.
            // */
            //Person(Actor::id_t id, PersonAttributes attribs, Date dob, std::weak_ptr<const Person> mother, Date conception_date);

            Person(const Person&) = delete;
            Person& operator=(const Person&) = delete;

            
            /** Return PersonAttributes. */
            const PersonAttributes& attributes() const {
                return _attribs;
            }

			/** @return Person's ethnicity */
			PersonAttributes::ethnicity_t ethnicity() const {
				return _attribs.ethnicity();
			}

			/** @return Person's sex */
			Sex sex() const {
				return _attribs.sex();
			}
            
            /** Return date of birth */
            Date date_of_birth() const {
                return _dob;
            }

			Date::year_type year_of_birth() const {
				return yob_;
			}
            
            /** Return age in years, rounded down to last birthday.
             * @param[in] as_of Date when age has to be calculated.
             * @return Age in years. E.g. for DOB = 4 June 1989 and as_of = 5 May 2019, return 29. Returns 0 before date of birth.
             */
            unsigned int age(Date as_of) const;
            
            /** Calculate age as year fraction (without rounding).
             * @param[in] as_of Date when age has to be calculated.
             * @return Age as fraction. E.g. for DOB = 4 June 1989 and as_of = 4 Sep 2019, return ~30.25. Returns 0 before date of birth.
             */
            double age_fract(Date as_of) const;

            /** Has the person died yet. */
            bool died() const {
                return !_dod.is_not_a_date();
            }

            /** Is the person alive at the moment. 
             * @param[in] as_of current date
             * @return True if current date is on or past date of birth and before date of death.
             */
            bool is_alive(Date as_of) const {
				return as_of >= _dob && (_dod.is_not_a_date() || as_of < _dod);
            }
            
            /** Mark the person as deceased.  Can be called more than once.
             * @param[in] date Valid normal date.
             * @return Reference to person.
             */
            Person& die(Date date);

			/** Date of death. NAD if it wasn't set yet. */
			Date date_of_death() const {
				return _dod;
			}

            const HistoryFactoryRegistry<Person>& get_history_registry(const ImmutableContext& im_ctx) const override;

            /** Person's mother (nullptr if not set) */
            const Person::const_weak_ptr& mother() const {
                return _mother;
            }

            /** Conception date of the person. Returns not-a-date if not set. */
            Date conception_date() const {
                return _conception_date;
            }

            typedef unsigned int child_idx_t;

            /** Number of this person's children */
            child_idx_t nbr_children() const;

            /** Get the idx-th child 
              @return Pointer which can be null if only date of birth is stored.
              @throw std::out_of_range If idx >= nbr_children()
             */
            Person::shared_ptr get_child(child_idx_t idx);

            /** Get the idx-th child
              @return non-null pointer
              @throw std::out_of_range If idx >= nbr_children_born()
             */
            Person::const_shared_ptr get_child(child_idx_t idx) const;

			/** Get date of birth of idx-th child. 
			@throw std::out_of_range If idx >= nbr_children_born()
			*/
			Date get_child_birth_date(child_idx_t idx) const;

            /** Add a new fetus.
              @throw std::logic_error If the person is not female. If the conception date of the fetus being added is before the conception date of the previously added fetus.
            */
            void add_fetus(const Fetus& fetus);

            /** Remove fetuses conceived before this date (due to miscarriage or abortion). Does nothing on male persons.
             */
            void remove_fetuses(Date date);

			/** Number of this person's fetuses */
			child_idx_t nbr_fetuses() const;

            /** Give birth to fetuses conceived before this date. Does nothing on male persons.
              Children are stored in the order of birth.
              @param date Birth date
              @param ctx Contexts
              @param this_mother Shared pointer to this, to be passed to created children (HACK).
              TODO: remove this_mother argument
             */
            void give_birth(Date date, const Contexts& ctx, Person::shared_ptr this_mother);

			/** Add a child birth event (without storing the pointer to child).
			@throw std::logic_error If this type of Person doesn't store children (e.g. male persons in
			a model which only tracks motherhood). If the same child is being added more than once.
			*/
			void add_childbirth(Date child_date_of_birth);

            ///** Clone and set a new ID. Copies Person histories, but does not copy the links to mother and children. Copy additional childbirths (without child link).
            //*/
            //std::unique_ptr<Person> partial_clone(Actor::id_t new_id) const;

			/** Check if this Person is a mother of other. */
			bool is_parent_of(const Person& other) const;

            /** Set up links between parents and child. 
              @param child Pointer to child. Cannot be null.
              @param mother Pointer to mother. Cannot be null. Must be female.
              @param conception_date Conception date. Must be valid, after mother's date of birth and before child's date of birth.
              @throw std::logic_error If mother and conception date have been already set on the child. If the same child is being added more than once.
              @throw std::domain_error If any parameter is not valid.
             */
            static void link_parents_child(const Person::shared_ptr child, Person::shared_ptr mother, Date conception_date);            

            /** Set up links between parents and children.
            @param persons Simulation objects which are lacking mother-child links. Pointers cannot be null. Assumed to be sorted by IDs.
            @param person_data Data from which persons were created. They do not need to have their histories. Assumed to be sorted by IDs.
            @throw std::domain_error If vectors have different sizes. If there is no valid pointer to a Person with child or mother ID.
            */
            static void link_parents_child(std::vector<Person::shared_ptr>& persons, const std::vector<PersonData>& person_data);

            /** Remove links to children born or on after "since" */
            void unlink_children(Date since);

			/** Convert to a pure data object. Child IDs are added in the same order as they are in Person object.			
			*/
			PersonData to_data(const ImmutableContext& im_ctx) const;

			static bool compare_ptr_by_id(const Person::shared_ptr& l, const Person::shared_ptr& r) {
				assert(l);
				assert(r);
				return l->id() < r->id();
			}

			/** Predicate to set for equality by ID */
			static bool equal_ptr_by_id(const Person::shared_ptr& l, const Person::shared_ptr& r) {
				assert(l);
				assert(r);
				return l->id() == r->id();
			}

			/** Date of *last* immigration event 
			It is not cleared when emigrating!.
			*/
			Date immigration_date() const {
				return immigration_date_;
			}

			/** @throw std::domain_error If immigration date is already set, or if imdate is before date of birth or after date of death, or imdate is a "special" date. */
			void set_immigration_date(Date imdate);
        private:
            void sort_childbirths();
			void add_child(Person::shared_ptr child);
            void set_parents_data(Person::const_weak_ptr mother, Date conception_date);
        private:
            typedef std::pair<Date, Person::shared_ptr> child_t; /**< Stores information about Person's child. Date of birth is stored separately
                                                                      from the pointer so that we can store birth events from before the simulation without
                                                                      necessarily creating Person objects for resulting children. */
            typedef std::vector<child_t> child_vec_t;
            typedef std::vector<Fetus>  fetus_vec_t;

            Person::const_weak_ptr _mother; /**< Mother */
            std::unique_ptr<child_vec_t> _children; /**< Children vector. Null for male Persons. */
            std::unique_ptr<fetus_vec_t> _fetuses; /**< Fetuses vector. Null for male Persons. */
			PersonAttributes _attribs; /**< person fixed attributes */
			Date::year_type yob_; /**< Year of birth (cached) */
            Date _dob; /**< date of birth */			
            Date _dod; /**< date of death */
            Date _conception_date; /**< Date the person was conceived */
			Date immigration_date_; /**< Date of last immigration event */			
        };
    }
}

#endif
