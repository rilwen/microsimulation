// (C) Averisera Ltd 2014-2020
#pragma once
#include "core/dates.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace averisera {
	namespace microsim {
		class Contexts;
		class Person;
		template <class T> class Predicate;

		/** Selects alive persons from the emigrant population stored in MutableContext. Does not modify the context. */
		class EmigrantSelector {
		public:
			/** Select emigrants who left the country between dates given below and are selected by the predicate
			@param predicate Select emigrants for each emigration date covered using this predicate 
			@param from First emigration date (inclusive)
			@param to Last emigration date (exclusive) 
			@param min_age Minimum age of returning emigrants
			@throw std::domain_error If predicate is null
			*/
			EmigrantSelector(std::shared_ptr<const Predicate<Person>> predicate, Date from, Date to, unsigned int min_age);

			/** Select emigrants. 
			@param[in] asof Simulation date when selection is made
			@param[in] number Number of emigrants to select
			@param[out] selected Maps emigration date -> selected emigrants. Old contents are removed.
			*/
			void select(const Contexts& ctx, Date asof, std::unordered_map<Date, std::vector<std::shared_ptr<Person>>>& selected) const;

			/** Select emigrants.
			@param[in] asof Simulation date when selection is made
			@param[in] number Number of emigrants to select
			@param[out] selected Vector for selected emigrants. Old contents are removed.
			*/
			void select(const Contexts& ctx, Date asof, std::vector<std::shared_ptr<Person>>& selected) const;

			unsigned int min_age() const {
				return min_age_;
			}

			/** Describe predicate as string*/
			std::string predicate_as_string() const;
		private:
			std::shared_ptr<const Predicate<Person>> predicate_;
			Date from_; /**< inclusive */
			Date to_; /**< exclusive*/
			unsigned int min_age_;
		};
	}
}
