#ifndef __AVERISERA_MS_POPULATION_H
#define __AVERISERA_MS_POPULATION_H

#include <algorithm>
#include <memory>
#include <vector>
#include "actor.hpp"

namespace averisera {
    namespace microsim {
        class Contexts;
        class Person;
        struct PersonData;
        class Population;        
        struct PopulationData;
        template <class T> class Predicate;

        namespace {
            template <class T> const std::vector<std::shared_ptr<T>>& get_members(const Population& population);
            template <> const std::vector<Actor::shared_ptr<Person>>& get_members(const Population& population);
        }
        
        /** @brief Simulated population
         * 
         * Holds the entire population 
         */
        class Population {
        public:
			Population(const std::string& name = std::string());
            Population(const Population&) = delete;
            Population& operator=(const Population&) = delete;

            /** Move constructor */
            Population(Population&& other);

            /* Import Person objects from PersonData. Moves as much data as possible to save memory. 
			Sorted added Person objects by ID in ascending order
            @param keep_ids If false, reset IDs.    
			@param immigration This is immigration
			*/
            void import_data(PopulationData& data, const Contexts& ctx, bool keep_ids, bool immigration);
            
            /** Get person with given ID.
             * @return null if not found
*/
            Actor::shared_ptr<Person> get_person(Actor::id_t id) const;
            
            const std::vector<Actor::shared_ptr<Person>>& persons() const {
                return _persons;
            }

			/** Return a vector with pointers to persons who are alive at asof date. */
			std::vector<Actor::shared_ptr<Person>> live_persons(Date asof) const;

            /** For templated access to members */
            template <class T> const std::vector<std::shared_ptr<T>>& members() const {
                return get_members<T>(*this);
            }

            /** Merge in elements of the other population
              @throw std::logic_error If populations have two same persons or with the same IDs
             */
            void merge(const Population& other);

            /** Wipe out the population */
            void wipe_out();

            /** Transfer selected persons from source population to this. 
              @param source Source Population
              @param selector Return true on persons selected for transfer
              @throw std::logic_error If this population and selected persons from other population have two same persons or with the same IDs
             */
            void transfer_persons(Population& source, const Predicate<Person>& selector, const Contexts& ctx);            

            /** Find a shared pointer to an Actor-derived object in a vector sorted by IDs. We assume no null pointers.
            @tparam AD Derived from ActorImpl<T>.
            */
            template <class AD> static typename AD::shared_ptr find_by_id(const std::vector<typename AD::shared_ptr>& persons, Actor::id_t id);

            /** Add new person.
            * @throw std::domain_error If new person does not have higher ID than other persons in the population, or pointer is null.
            */
            void add_person(Actor::shared_ptr<Person> person, bool check_id = true);

            /** Add new persons.
			@param persons Vector of new persons, sorted by ID in ascending order.
            * @throw std::domain_error If one of the added pointers is null or persons is not sorted by ID in ascending order
            */
            void add_persons(const std::vector<Actor::shared_ptr<Person>>& persons, bool check_ids = true);

			/** Remove this persons.
			@param persons Vector of removed persons, sorted by ID in ascending order.
			@throw std::domain_error If any person in persons is not a member of this population, or is a null pointer.
			*/
			void remove_persons(const std::vector<Actor::shared_ptr<Person>>& persons);

			/** Is population empty? */
			bool empty() const;

			const std::string& name() const {
				return name_;
			}

			/** Sort Person vector by ID in ascending order */
			static void sort_persons(std::vector<Actor::shared_ptr<Person>>& persons);			
        private:
			static void merge_persons(std::vector<Actor::shared_ptr<Person>>& dst, const std::vector<Actor::shared_ptr<Person>>& src);
            static void link_parents_children(std::vector<Actor::shared_ptr<Person>>& persons, const std::vector<PersonData>& person_data);

            /* Import Person objects from PersonData. Moves as much data as possible to save memory.
            */
            void import_persons(std::vector<PersonData>& person_datas, const Contexts& ctx, bool keep_ids, bool immigration);

			/** Sort this persons by ID in ascending order */
			void sort_persons() {
				sort_persons(_persons);
			}

			std::string name_;
            std::vector<Actor::shared_ptr<Person>> _persons; /**< Persons sorted by ID */
        };

        template <class AD> typename AD::shared_ptr Population::find_by_id(const std::vector<typename AD::shared_ptr>& objects, Actor::id_t id) {
			const auto iter = Actor::find_by_id<AD>(objects, id);
            if (iter != objects.end()) {
                return *iter;
            } else {
                return nullptr;
            }
        }

        namespace {
            template <> inline const std::vector<Actor::shared_ptr<Person>>& get_members<Person>(const Population& population) {
                return population.persons();
            }
        }
    }
}

#endif // __AVERISERA_MS_POPULATION_H
