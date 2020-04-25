/*
  (C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_PREDICATE_FACTORY_H
#define __AVERISERA_MS_PREDICATE_FACTORY_H

#include "microsim-core/cohort.hpp"
#include "microsim-core/person_attributes.hpp"
#include "microsim-core/pregnancy.hpp"
#include <initializer_list>
#include <memory>
#include <vector>

namespace averisera {
	namespace microsim {		
		class Person;
        template <class T> class Predicate;        
		enum class Sex : uint8_t;

		/** @brief Functions to make predicates.

         */
		namespace PredicateFactory {
            /** @see PredAnd */
			template <class T> std::unique_ptr<const Predicate<T>> make_and(std::initializer_list<std::shared_ptr<const Predicate<T>>> predicates);

			/** @see PredAnd */
			template <class T> std::unique_ptr<const Predicate<T>> make_and(std::unique_ptr<const Predicate<T>>&& p1, std::unique_ptr<const Predicate<T>>&& p2);

			/** @see PredAnd */
			template <class T> std::unique_ptr<const Predicate<T>> make_and(std::shared_ptr<const Predicate<T>> p1, std::shared_ptr<const Predicate<T>> p2) {
				return make_and({ p1, p2 });
			}

            /** @see PredOr */
			template <class T> std::unique_ptr<const Predicate<T>> make_or(std::initializer_list<std::shared_ptr<const Predicate<T>>> predicates);

            /** @see PredOr */
			template <class T> std::unique_ptr<const Predicate<T>> make_or(const std::vector<std::shared_ptr<const Predicate<T>>>& predicates);

            /** @see PredOr */
			template <class T> std::unique_ptr<const Predicate<T>> make_or(std::vector<std::shared_ptr<const Predicate<T>>>&& predicates);
			
            /** @see PredNot */
			template <class T> std::unique_ptr<const Predicate<T>> make_not(std::shared_ptr<const Predicate<T>> pred);

			/** @see PredNot */
			template <class T> std::unique_ptr<const Predicate<T>> make_not(std::unique_ptr<const Predicate<T>>&& pred) {
				return make_not(std::shared_ptr<const Predicate<T>>(std::move(pred)));
			}

            /** @see PredYearOfBirth */
			std::unique_ptr<const Predicate<Person>> make_year_of_birth(int min_year, int max_year, bool alive = true);

            /** @see PredAge */
            std::unique_ptr<const Predicate<Person>> make_age(unsigned int min_age, unsigned int max_age, bool alive = true);

			/** @see PredAge */
			std::unique_ptr<const Predicate<Person>> make_min_age(unsigned int min_age, bool alive = true);

			/** @see PredAge */
			std::unique_ptr<const Predicate<Person>> make_max_age(unsigned int max_age, bool alive = true);

            /** @see PredEthnicity */
			std::unique_ptr<const Predicate<Person>> make_ethnicity(PersonAttributes::ethnicity_t from, PersonAttributes::ethnicity_t to, bool alive);

			/** @see PredEthnicity */
			std::unique_ptr<const Predicate<Person>> make_ethnicity(std::unordered_set<PersonAttributes::ethnicity_t>&& allowed, bool alive);

			/** @see PredEthnicity */
			std::unique_ptr<const Predicate<Person>> make_ethnicity(const std::unordered_set<PersonAttributes::ethnicity_t>& allowed, bool alive);

            /** @see PredSex */
			std::unique_ptr<const Predicate<Person>> make_sex(Sex sex, bool alive = true);

			/** @see PredSex 
			Returns a shared pointer to save memory
			*/
			std::shared_ptr<const Predicate<Person>> make_sex_shared(Sex sex, bool alive);

            /** @see PredAlive */
            std::unique_ptr<const Predicate<Person>> make_alive();

            /** @see PredPregnancy */
            std::unique_ptr<const Predicate<Person>> make_pregnancy(Pregnancy::State state, bool alive, bool at_start);

			/** @see PredImmigrationDate */
			std::unique_ptr<const Predicate<Person>> make_immigration_date(Date from, Date to, bool allow_non_immigrants, bool require_alive);
                        
            /** Make a Predicate which always returns true 
            @see PredTrue
            */
            template <class T> std::unique_ptr<const Predicate<T>> make_true();

            /** @see PredVariableRange */
            template <class T, class V> std::unique_ptr<const Predicate<T>> make_variable_range(const std::string& variable, V min, V max, bool accept_missing = false);            

			/** @see PredAsof */
			template <class T> std::unique_ptr<const Predicate<T>> make_asof(Date begin, Date end);

			/** Make a predicate selecting a person from a cohort
			@param alive If true, select only alive persons
			@param min_age Minimum age (0 for no minimum)
			*/
			std::unique_ptr<const Predicate<Person>> make_cohort(const std::map<std::string, std::shared_ptr<const Predicate<Person>>> ethnic_predicates, Cohort::yob_ethn_sex_cohort_type cohort, bool alive, unsigned int min_age);
		}
    }
}

//#include <type_traits>
//#include "actor.h"
#include "predicate/pred_and.hpp"
#include "predicate/pred_or.hpp"
#include "predicate/pred_not.hpp"
#include "predicate/pred_true.hpp"
#include "predicate/pred_variable_range.hpp"
#include "predicate/pred_asof.hpp"

namespace averisera {
	namespace microsim {

        // Implementations of template functions
        namespace PredicateFactory {
			template <class T> std::unique_ptr<const Predicate<T>> make_and(std::initializer_list<std::shared_ptr<const Predicate<T>>> predicates) {
				return std::unique_ptr<const Predicate<T>>(new PredAnd<T>(predicates));
			}

			template <class T> std::unique_ptr<const Predicate<T>> make_and(std::unique_ptr<const Predicate<T>>&& p1, std::unique_ptr<const Predicate<T>>&& p2) {
				const std::shared_ptr<const Predicate<T>> sp1(std::move(p1));
				const std::shared_ptr<const Predicate<T>> sp2(std::move(p2));
				return make_and({ sp1, sp2 });
			}

			template <class T> std::unique_ptr<const Predicate<T>> make_or(std::initializer_list<std::shared_ptr<const Predicate<T>>> predicates) {
				return std::unique_ptr<const Predicate<T>>(new PredOr<T>(predicates));
			}
            
			template <class T> std::unique_ptr<const Predicate<T>> make_or(const std::vector<std::shared_ptr<const Predicate<T>>>& predicates) {
				return std::unique_ptr<const Predicate<T>>(new PredOr<T>(predicates));
			}
            
			template <class T> std::unique_ptr<const Predicate<T>> make_or(std::vector<std::shared_ptr<const Predicate<T>>>&& predicates) {
				return std::unique_ptr<const Predicate<T>>(new PredOr<T>(std::move(predicates)));
			}
			
			template <class T> std::unique_ptr<const Predicate<T>> make_not(std::shared_ptr<const Predicate<T>> pred) {
				return std::unique_ptr<const Predicate<T>>(new PredNot<T>(pred));
			}

            template <class T> std::unique_ptr<const Predicate<T>> make_true() {
                return std::unique_ptr<const Predicate<T>>(new PredTrue<T>());
            }

            template <class T, class V> std::unique_ptr<const Predicate<T>> make_variable_range(const std::string& variable, V min, V max, bool accept_missing) {
                //static_assert(std::is_base_of<Actor, T>::value, "T must be derived from Actor");
                return std::unique_ptr<const Predicate<T>>(new PredVariableRange<T, V>(variable, min, max, accept_missing));
            }

			template <class T> std::unique_ptr<const Predicate<T>> make_asof(Date begin, Date end) {
				return std::unique_ptr<const Predicate<T>>(new PredAsof<T>(begin, end));
			}
		}
	}
}

#endif 
