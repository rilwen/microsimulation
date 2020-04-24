/*
  (C) Averisera Ltd 2015
*/
#include <memory>
#include "predicate_factory.hpp"
#include "predicate/pred_year_of_birth.hpp"
#include "predicate/pred_age.hpp"
#include "predicate/pred_ethnicity_single.hpp"
#include "predicate/pred_ethnicity_range.hpp"
#include "predicate/pred_ethnicity_set.hpp"
#include "predicate/pred_sex.hpp"
#include "predicate/pred_alive.hpp"
#include "predicate/pred_pregnancy.hpp"
#include "predicate/pred_immigration_date.hpp"

namespace averisera {
	namespace microsim {
        namespace PredicateFactory {
            std::unique_ptr<const Predicate<Person>> make_year_of_birth(int min_year, int max_year, bool alive) {
                return std::unique_ptr<const Predicate<Person>>(new PredYearOfBirth(min_year, max_year, alive));
            }

            std::unique_ptr<const Predicate<Person>> make_age(unsigned int min_age, unsigned int max_age, bool alive) {
                return std::unique_ptr<const Predicate<Person>>(new PredAge(min_age, max_age, alive));
            }

			std::unique_ptr<const Predicate<Person>> make_min_age(const unsigned int min_age, const bool alive) {
				return make_age(min_age, std::numeric_limits<unsigned int>::max(), alive);
			}

			std::unique_ptr<const Predicate<Person>> make_max_age(unsigned int max_age, bool alive) {
				return make_age(0u, max_age, alive);
			}

            std::unique_ptr<const Predicate<Person>> make_ethnicity(PersonAttributes::ethnicity_t from, PersonAttributes::ethnicity_t to, bool alive) {
				const PredEthnicity* pred = nullptr;
				if (from == to) {
					pred = new PredEthnicitySingle(from, alive);
				} else {
					pred = new PredEthnicityRange(from, to, alive);
				}
				assert(pred);
                return std::unique_ptr<const Predicate<Person>>(pred);
            }

			std::unique_ptr<const Predicate<Person>> make_ethnicity(std::unordered_set<PersonAttributes::ethnicity_t>&& allowed, bool alive) {
				const PredEthnicity* pred = nullptr;
				const size_t n = allowed.size();
				check_that(n > 0, "PredicateFactory: allowed set cannot be empty");
				if (n == 1) {
					pred = new PredEthnicitySingle(*allowed.begin(), alive);
				} else {
					const auto min_idx = *std::min_element(allowed.begin(), allowed.end());
					const auto max_idx = *std::max_element(allowed.begin(), allowed.end());
					if (n == static_cast<size_t>(max_idx - min_idx + 1)) {
						pred = new PredEthnicityRange(min_idx, max_idx, alive);
						allowed.clear(); // for consistency
					} else {
						pred = new PredEthnicitySet(std::move(allowed), alive);
					}
				}
				assert(pred);
				return std::unique_ptr<const Predicate<Person>>(pred);
			}

			std::unique_ptr<const Predicate<Person>> make_ethnicity(const std::unordered_set<PersonAttributes::ethnicity_t>& allowed, bool alive) {
				const PredEthnicity* pred = nullptr;
				const size_t n = allowed.size();
				check_that(n > 0, "PredicateFactory: allowed set cannot be empty");
				if (n == 1) {
					pred = new PredEthnicitySingle(*allowed.begin(), alive);
				} else {
					const auto min_idx = *std::min_element(allowed.begin(), allowed.end());
					const auto max_idx = *std::max_element(allowed.begin(), allowed.end());
					if (n == static_cast<size_t>(max_idx - min_idx + 1)) {
						pred = new PredEthnicityRange(min_idx, max_idx, alive);
					} else {
						pred = new PredEthnicitySet(allowed, alive);
					}
				}
				assert(pred);
				return std::unique_ptr<const Predicate<Person>>(pred);
			}

            std::unique_ptr<const Predicate<Person>> make_sex(Sex sex, bool alive) {
                return std::unique_ptr<const Predicate<Person>>(new PredSex(sex, alive));
            }

			static const std::shared_ptr<const Predicate<Person>> SEX_MALE(new PredSex(Sex::MALE, false));
			static const std::shared_ptr<const Predicate<Person>> SEX_MALE_ALIVE(new PredSex(Sex::MALE, true));
			static const std::shared_ptr<const Predicate<Person>> SEX_FEMALE(new PredSex(Sex::FEMALE, false));
			static const std::shared_ptr<const Predicate<Person>> SEX_FEMALE_ALIVE(new PredSex(Sex::FEMALE, true));

			std::shared_ptr<const Predicate<Person>> make_sex_shared(Sex sex, bool alive) {
				if (sex == Sex::MALE) {
					return alive ? SEX_MALE_ALIVE : SEX_MALE;
				} else {
					assert(sex == Sex::FEMALE);
					return alive ? SEX_FEMALE_ALIVE : SEX_FEMALE;
				}
			}

            std::unique_ptr<const Predicate<Person>> make_alive() {
                return std::unique_ptr<const Predicate<Person>>(new PredAlive());
            }

            std::unique_ptr<const Predicate<Person>> make_pregnancy(Pregnancy::State state, bool alive, bool at_start) {
                return std::unique_ptr<const Predicate<Person>>(new PredPregnancy(state, alive, at_start));
            }

			std::unique_ptr<const Predicate<Person>> make_immigration_date(Date from, Date to, bool allow_non_immigrants, bool require_alive) {
				return std::unique_ptr<const Predicate<Person>>(new PredImmigrationDate(from, to, allow_non_immigrants, require_alive));
			}

			std::unique_ptr<const Predicate<Person>> make_cohort(const std::map<std::string, std::shared_ptr<const Predicate<Person>>> ethnic_predicates, const Cohort::yob_ethn_sex_cohort_type cohort, const bool alive,
				const unsigned int min_age) {
				const auto yob = std::get<0>(cohort);
				const std::string& ethn_grp = std::get<1>(cohort);
				const Sex sex = std::get<2>(cohort);
				const auto ethn_grp_it = ethnic_predicates.find(ethn_grp);
				check_that(ethn_grp_it != ethnic_predicates.end(), "PredicateFactory::make_cohort: uknown ethnic grouping");
				const std::shared_ptr<const Predicate<Person>> pred_ethn = ethn_grp_it->second;
				const std::shared_ptr<const Predicate<Person>> pred_sex = make_sex_shared(sex, alive);
				const std::shared_ptr<const Predicate<Person>> pred_yob(PredicateFactory::make_year_of_birth(yob, yob, alive));
				std::unique_ptr<const Predicate<Person>> predicate(PredicateFactory::make_and({ pred_ethn, pred_sex, pred_yob }));
				if (min_age) {
					predicate = PredicateFactory::make_and(std::move(predicate), PredicateFactory::make_age(min_age, std::numeric_limits<unsigned int>::max() - 1, alive));
				}
				return predicate;
			}
        }
	}
}
