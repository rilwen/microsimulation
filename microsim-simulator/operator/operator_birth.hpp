#ifndef __AVERISERA_MICROSIM_OPERATOR_BIRTH_HPP
#define __AVERISERA_MICROSIM_OPERATOR_BIRTH_HPP

#include "../history_user_simple.hpp"
#include "../operator_individual.hpp"

namespace averisera {
    namespace microsim {
        class Person;

        /*! Handles birth: creating new Person objects from Fetus objects */
        class OperatorBirth: public OperatorIndividual<Person> {
        public:
            /*!
              \param pred Predicate. If null, it is replaced by a predicate selecting live females.
            */
            OperatorBirth(std::shared_ptr<const Predicate<Person>> pred = nullptr);

			/*!
			Generates appropriate predicate selecting women of childbearing age
			\param min_childbearing_age Minimum childbearing age in years
			\param max_childbearing_age Maximum childbearing age in years
			\throw std::domain_error If min_childbearing_age > max_childbearing_age.
			*/
			OperatorBirth(unsigned int min_childbearing_age, unsigned int max_childbearing_age);
            
            const Predicate<Person>& predicate() const override {
                return *_pred;
            }			

            void apply(const std::shared_ptr<Person>& obj, const Contexts& contexts) const override;

			const HistoryUser<Person>::use_reqvec_t& user_requirements() const override {
				return history_user_.user_requirements();
			}

			const std::string& name() const override {
				static const std::string str("Birth");
				return str;
			}
        private:
			HistoryUserSimple<Person> history_user_;
            std::shared_ptr<const Predicate<Person>> _pred;            
        };
    }
}

#endif // __AVERISERA_MICROSIM_OPERATOR_BIRTH_HPP
