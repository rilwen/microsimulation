#ifndef __AVERISERA_MICROSIM_OPERATOR_CONCEPTION_HPP
#define __AVERISERA_MICROSIM_OPERATOR_CONCEPTION_HPP

#include "../hazard_rate_multiplier_provider.hpp"
#include "../history_generator_simple.hpp"
#include "../history_user_simple.hpp"
#include "../operator_individual.hpp"
#include "microsim-core/conception.hpp"
#include "core/period.hpp"
#include <memory>
#include <vector>

namespace averisera {
    namespace microsim {
        class Person;
		template <class T> class HazardRateMultiplierProvider;
        template <class T> class RelativeRisk;		

        /*! Operator describing the conception (start of pregnancy). It can only generate a single conception event until the next schedule date. It will select females which can get pregnant during current schedule period
         */
        class OperatorConception: public OperatorIndividual<Person> {
        public:            			
            /*! 
              \param[in] conception Conception model
              \param[in] relative_risk Relative risks vector (elements can't be null).
              \param[in] external_predicate Predicate which selects Persons to act on from among non-pregnant females of appropriate age. If null, it is replaced by PredTrue. The Predicate provided is multiplied internally by predicates selecting sex, age and pregnancy status. 
              \param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule.
			  \param min_childbearing_age Minimum childbearing age in years
			  \param max_childbearing_age Maximum childbearing age in years
			  \param zero_fertility_period Period after childbirth during which there can be no conception
              \throw std::domain_error If any relative risk is null. If min_childbearing_age > max_childbearing_age. If zero_fertility_period has negative size.
             */
			OperatorConception(const Conception& conception, const std::vector<std::shared_ptr<const RelativeRisk<Person>>>& relative_risks, std::shared_ptr<const Predicate<Person>> external_predicate, std::unique_ptr<Schedule>&& schedule, unsigned int min_childbearing_age, unsigned int max_childbearing_age, Period zero_fertility_period);

            OperatorConception(const Conception& conception, std::shared_ptr<const Predicate<Person>> external_predicate, std::unique_ptr<Schedule>&& schedule, unsigned int min_childbearing_age, unsigned int max_childbearing_age, Period zero_fertility_period)
                : OperatorConception(conception, std::vector<std::shared_ptr<const RelativeRisk<Person>>>(), external_predicate, std::move(schedule), min_childbearing_age, max_childbearing_age, zero_fertility_period)
                {
            }

            const Predicate<Person>& predicate() const override {
                return *_pred;
            }
            
            void apply(const std::shared_ptr<Person>& obj, const Contexts& contexts) const override;

            bool active(Date date) const override {
                return Operator<Person>::active(_schedule, date);
            }

			const HistoryGenerator<Person>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const HistoryUser<Person>::use_reqvec_t& user_requirements() const override {
				return hist_use_.user_requirements();
			}

			OperatorConception& add_hazard_rate_multiplier_provider(std::unique_ptr<const HazardRateMultiplierProvider<Person>>&& new_provider) {
				check_not_null(new_provider, "OperatorConception: hazard rate multiplier provider cannot be null");
				hrm_providers_.push_back(std::move(new_provider));
				return *this;
			}

			const std::string& name() const override {
				static const std::string str("Conception");
				return str;
			}

			/*! Calculate first date when conception can happen */
			static Date calc_first_conception_date_allowed(const std::shared_ptr<Person>& obj, const Contexts& ctx, unsigned int min_childbearing_age, Period zero_fertility_period);
        private:			
			HistoryUserSimple<Person> hist_use_;
			HistoryGeneratorSimple<Person> hist_gen_;
            Conception _conception;            
            std::vector<std::shared_ptr<const RelativeRisk<Person>>> _relative_risks;
            std::shared_ptr<const Predicate<Person>> _pred;
            std::unique_ptr<Schedule> _schedule;
			std::vector<std::unique_ptr<const HazardRateMultiplierProvider<Person>>> hrm_providers_;
			unsigned int min_childbearing_age_;
			unsigned int max_childbearing_age_;
			Period zero_fertility_period_;

			static std::shared_ptr<const Predicate<Person>> predicate_for_history_generator();

			/*! Add required conditions particular to OperatorConception */
			static std::shared_ptr<const Predicate<Person>> predicate_for_operator(const std::shared_ptr<const Predicate<Person>>& external_predicate, unsigned int min_childbearing_age, unsigned int max_childbearing_age);

			Date calc_first_conception_date_allowed(const std::shared_ptr<Person>& obj, const Contexts& ctx) const;
        };
    }
}

#endif // __AVERISERA_MICROSIM_OPERATOR_CONCEPTION_HPP
