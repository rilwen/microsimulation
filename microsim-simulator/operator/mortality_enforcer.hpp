#ifndef __AVERISERA_MORTALITY_ENFORCER_H
#define __AVERISERA_MORTALITY_ENFORCER_H

#include "../operator.hpp"

namespace averisera {
    template <class T> class GenericDistribution;

    namespace microsim {
        class Person;

        /*! \brief Operator which enforces a predefined percentage of dead persons
         */
        class MortalityEnforcer: public Operator<Person> {
        public:
            /*! 
              \param[in] distributions Vector of GenericDistribution<bool> implementations for IsDead status (true = dead).
              \throw std::domain_error If predicate or any of the distributions is null or has wrong value range.
             */
            MortalityEnforcer(std::shared_ptr<const Predicate<Person>> predicate, const std::vector<std::shared_ptr<const GenericDistribution<bool>>>& distributions);

            const Predicate<Person>& predicate() const override {
                return *_predicate;
            }
    
            /*! \throw std::out_of_range If not enough distributions */
            void apply(const std::vector<std::shared_ptr<Person>>& selected, const Contexts& contexts) const override;

			const std::string& name() const override {
				static const std::string str("MortalityEnforcer");
				return str;
			}
        private:
            std::shared_ptr<const Predicate<Person>> _predicate;            
            std::vector<std::shared_ptr<const GenericDistribution<bool>>> _distributions;
        };
    }
}

#endif // __AVERISERA_MORTALITY_ENFORCER_H
