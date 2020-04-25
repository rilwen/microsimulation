#pragma once
#include <memory>
#include <vector>

namespace averisera {
	namespace microsim {
		class Contexts;
		class Person;

		/** Select persons who will leave the main population (without replacement) */
		class MigrantSelector {
		public:
			virtual ~MigrantSelector();

			/** Select migrants. Assumes all persons provided in source are currently alive.
			@param source Vector with source population from which migrants have to be selected. Vector can be modified at will.
			@param migrants Add selected pointers to this vector, preserving the existing ones.
			@param number How many migrants to select
			*/
			void select(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& source, std::vector<std::shared_ptr<Person>>& migrants, size_t number) const;
		private:
			virtual void select_impl(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& source, size_t number, std::vector<std::shared_ptr<Person>>::iterator dest_begin) const = 0;
		};
	}
}
