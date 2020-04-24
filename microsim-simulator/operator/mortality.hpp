#ifndef __AVERISERA_MORTALITY_OPERATOR_H
#define __AVERISERA_MORTALITY_OPERATOR_H

#include "../person.hpp"
#include "operator_hazard_model.hpp"

namespace averisera {
	class CSVFileReader;

    namespace microsim {
		class Person;
		template <class T> class Predicate;
        /*! \brief Mortality operator 

          Based on HazardModel.
         */
        class Mortality: public OperatorHazardModel<Person> {
        public:
            static const unsigned int ALIVE = 0;
            static const unsigned int DEAD = 1;

            /*! \see OperatorHazardModel 
			\param move_to_birth_date If true, move the HazardModel to Person's birth date before querying for probability of death.
              \throw std::domain_error If hazard_model.dim() != 2
             */
            Mortality(const HazardModel& hazard_model, const std::vector<std::shared_ptr<const RelativeRisk<Person>>>& relative_risks, std::shared_ptr<const Predicate<Person>> predicate, std::unique_ptr<Schedule>&& schedule, bool move_to_birth_date);

			/*! \see OperatorHazardModel
			\param move_to_birth_date If true, move the HazardModel to Person's birth date before querying for probability of death.
			\throw std::domain_error If hazard_model.dim() != 2
			*/
			Mortality(HazardModel&& hazard_model, std::vector<std::shared_ptr<const RelativeRisk<Person>>>&& relative_risks, std::shared_ptr<const Predicate<Person>> predicate, std::unique_ptr<Schedule>&& schedule, bool move_to_birth_date);

			const std::string& name() const override {
				static const std::string str("Mortality");
				return str;
			}

			/*! 
			\param mortality_curves Vector of mortality curves, one for each birth year
			\param schedule Pointer to schedule. If schedule != nullptr, *schedule is copied to each created operator.
			\param predicate Predicate AND-ed with the year of birth predicate for each curve
			\throw std::domain_error If any mortality curve is null */
			static std::vector<std::unique_ptr<Mortality>> build_operators(std::vector<std::unique_ptr<AnchoredHazardCurve>>&& mortality_curves, const Schedule* schedule, const std::shared_ptr<const Predicate<Person>>& predicate);
        private:
            state_t current_state(const Person& person, const Contexts& ctx) const override;
            void set_next_state(Person& person, Date date, state_t state, const Contexts& ctx) const override;
			std::unique_ptr<HazardModel> adapt_hazard_model(const Person& obj) const override;

			bool _move_to_birth_date;
        };
    }
}

#endif // __AVERISERA_MORTALITY_OPERATOR_H
