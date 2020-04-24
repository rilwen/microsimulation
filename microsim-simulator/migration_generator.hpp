#pragma once
#include <memory>
#include <vector>

namespace averisera {
	class Date;
	namespace microsim {
		class Contexts;
		class Person;
		struct PersonData;
		class Population;
		struct SchedulePeriod;

		/*! Handles migration to/from Population. */
		class MigrationGenerator {
		public:
			virtual ~MigrationGenerator();

			/*! Select Person objects to be removed from the population or added to it to account for migration over the period from the current simulation date to the next.
			\param population Population
			\param ctx Contexts object with the simulation schedule.
			\param persons_removed[out] Resized to correct size and filled with non-null pointers to objects which should be removed from population.
			\param persons_added[out] New PersonData which should be used to create Person objects to be added to the population. Expected to have correct IDs.
			*/
			virtual void migrate_persons(const Population& population, const Contexts& ctx, std::vector<std::shared_ptr<Person>>& persons_removed, std::vector<PersonData>& persons_added) const = 0;

			/*! Name of the generator (for logging) */
			virtual const std::string& name() const = 0;

			/*! Calc date for migration transfers for given schedule period */
			static Date calc_migration_date(const SchedulePeriod& sp);

			/*! Migrate children below threshold age with their parents
			\param parents_begin_idx First index of the adults in original adults vector that we take into account as possible parents of comigrated children
			\param comigrated_child_age_limit Age (rounded down) below which children only migrate with their parents
			\return Number of co-migrated children */
			static size_t comigrate_children(std::vector<std::shared_ptr<Person>>& adults, Date migration_date, unsigned int comigrated_child_age_limit, size_t parents_begin_idx);

			/*! Convert persons added to the population to data format to clone them as new members. Modifies the vector persons_added_ptrs. Issues new IDs.
			\param[in] ctx Context
			\param[in] persons_added_ptrs Pointers to Person objects which are to be cloned
			\param[in] comigrated_child_age_limit Age (rounded down) below which children only migrate with their parents
			\param[out] persons_added_data Result 
			\param[in] immigration_date Date of migration
			*/
			static void convert_added_persons_to_data(const Contexts& ctx, std::vector<std::shared_ptr<Person>>& persons_added_ptrs, unsigned int comigrated_child_age_limit, std::vector<PersonData>& persons_added_data, Date immigration_date);
		protected:
			/*! Remove persons below min_age from the selected group and return the number of removed persons */
			static size_t remove_below_age(Date asof, unsigned int min_age, std::vector<std::shared_ptr<Person>>& selected);
		private:
			static std::vector<std::shared_ptr<Person>> comigrate_children(std::vector<std::shared_ptr<Person>>::iterator begin, std::vector<std::shared_ptr<Person>>::iterator end, Date migration_date, unsigned int comigrated_child_age_limit);

		};
	}
}
