#pragma once
#include "../predicate.hpp"

namespace averisera {
	namespace microsim {
		class Person;

		/** Selects persons based on the date of immigration. */
		class PredImmigrationDate : public Predicate<Person> {
		public:
			/**
			@param from First imigration date (inclusive) 
			@param to Last imigration date (exclusive); to >= from
			@param allow_non_immigrants Select non-immigrants as well
			@param require_alive Select only live people
			@throw std::domain_error If to < from
			*/
			PredImmigrationDate(Date from, Date to, bool allow_non_immigrants, bool require_alive);

			bool select(const Person& obj, const Contexts& contexts) const override;

			bool select_alive(const Person& obj, const Contexts& contexts) const override {
				return select_out_of_context(obj);
			}

			bool select_out_of_context(const Person& obj) const override;

			Predicate<Person>* clone() const override;

			void print(std::ostream& os) const override;

			bool active(Date) const override;

			bool selects_alive_only() const override {
				return require_alive_;
			}
		private:
			Date from_;
			Date to_;
			bool allow_non_immigrants_;
			bool require_alive_;
		};
	}
}
