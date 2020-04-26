// (C) Averisera Ltd 2014-2020
#pragma once
#include "../migration_generator.hpp"
#include "microsim-core/migration_model.hpp"
#include <utility>
#include <vector>

namespace averisera {
	namespace microsim {
		class MigrantSelector;
		template <class A> class Predicate;		

		/** Uses MigrationModel objects.
		*/
		class MigrationGeneratorModel : public MigrationGenerator {
		public:
			typedef std::pair<std::unique_ptr<const Predicate<Person>>, MigrationModel> pred_model_pair;
			
			/** 
			@param name Generator name
			@param comigrated_child_age_limit Mothers always migrate with children which are less than comigrated_child_age_limit years old (rounded down)
			@param migrant_selector Selects candidates for emigration
			@throw std::domain_error If any of the predicates is null  or emigrant_selector is null
			*/
			MigrationGeneratorModel(std::string&& name, std::vector<pred_model_pair>&& models, unsigned int comigrated_child_age_limit, std::shared_ptr<const MigrantSelector> migrant_selector);

			void migrate_persons(const Population& population, const Contexts& ctx, std::vector<std::shared_ptr<Person>>& persons_removed, std::vector<PersonData>& persons_added) const override;			

			const std::string& name() const override {
				return name_;
			}
		private:
			std::vector<pred_model_pair> models_;
			unsigned int comigrated_child_age_limit_;
			std::shared_ptr<const MigrantSelector> migrant_selector_;
			std::string name_;
		};
	}
}
