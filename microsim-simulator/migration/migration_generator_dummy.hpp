#pragma once
#include "../migration_generator.hpp"

namespace averisera {
	namespace microsim {
		/*! Do-nothing migration generator */
		class MigrationGeneratorDummy : public MigrationGenerator {
		public:
			void migrate_persons(const Population& population, const Contexts& ctx, std::vector<std::shared_ptr<Person>>& persons_removed, std::vector<PersonData>& persons_added) const override {}

			const std::string& name() const override {
				static const std::string name("DUMMY");
				return name;
			}
		};
	}
}
