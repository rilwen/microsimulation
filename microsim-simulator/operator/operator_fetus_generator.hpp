#ifndef __AVERISERA_MICROSIM_OPERATOR_FETUS_GENERATOR_HPP
#define __AVERISERA_MICROSIM_OPERATOR_FETUS_GENERATOR_HPP

#include "../history_user_simple.hpp"
#include "../operator_individual.hpp"
#include "microsim-core/conception.hpp"

namespace averisera {
    namespace microsim {
        class Fetus;
        class Person;

        /** Abstract operator which generates children conceived since asof but before the next simulation date. */
        class OperatorFetusGenerator: public OperatorIndividual<Person> {
        public:
            /**
              @param pred Predicate which selects Persons to act on from among non-pregnant females. If null, it is replaced by PredTrue. The Predicate provided is summed internally with PredSex(female) * PredAge(min_childbearing_age - 1, max_childbearing_age, true).
            */
            OperatorFetusGenerator(std::shared_ptr<const Predicate<Person>> pred, unsigned int min_childbearing_age, unsigned int max_childbearing_age);
            
            void apply(const std::shared_ptr<Person>& mother, const Contexts& contexts) const override;

			const HistoryUser<Person>::use_reqvec_t& user_requirements() const override {
				return history_user_.user_requirements();
			}

			const Predicate<Person>& predicate() const {
				return *_pred;
			}
        private:
            /** Return a vector of generated fetuses
              @param mother Mother
              @param conception_dates Conception date
              @param multiplicities Pregnancy multiplicity
              @param contexts
             */
            virtual std::vector<Fetus> generate_fetuses(const std::shared_ptr<Person>& mother, Date conception_date, Conception::multiplicity_type multiplicity, const Contexts& contexts) const = 0;

			HistoryUserSimple<Person> history_user_;
            std::shared_ptr<const Predicate<Person>> _pred;
			unsigned int min_childbearing_age_;
			unsigned int max_childbearing_age_;
        };
    }
}

#endif // __AVERISERA_MICROSIM_OPERATOR_FETUS_GENERATOR_HPP
