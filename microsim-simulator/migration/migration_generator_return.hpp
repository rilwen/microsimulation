#pragma once
#include "../migration_generator.hpp"
#include "emigrant_selector.hpp"

namespace averisera {
	namespace microsim {
		/** A given fraction of selected emigrants returns */
		class MigrationGeneratorReturn : public MigrationGenerator {
		public:
			/** 
			@param name Generator name
			@param from Start of Returning (inclusive)
			@param to End of returning (exclusive)
			@param fraction Fraction of emigrants returning in [0, 1]
			@param returning_children_age_limit Children below that age return with their mother only (they are not selected).
			@param emigrant_selector Emigrant selector
			@throw std::domain_error If emigrant_selector is null, from >= to or fraction is outside [0, 1] 
			*/
			MigrationGeneratorReturn(std::string&& name, Date from, Date to, double fraction, unsigned int returning_children_age_limit, std::unique_ptr<EmigrantSelector>&& emigrant_selector);
			
			void migrate_persons(const Population& population, const Contexts& ctx, std::vector<std::shared_ptr<Person>>& persons_removed, std::vector<PersonData>& persons_added) const override;

			const std::string& name() const override {
				return name_;
			}
		private:
			std::unique_ptr<EmigrantSelector> emigrant_selector_;
			Date from_;
			Date to_;
			double fraction_;
			unsigned int returning_children_age_limit_;
			std::string name_;			
		};
	}
}
