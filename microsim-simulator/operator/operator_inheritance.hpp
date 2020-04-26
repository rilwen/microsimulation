// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_OPERATOR_INHERITANCE_H
#define __AVERISERA_MS_OPERATOR_INHERITANCE_H

#include "../history_generator_simple.hpp"
#include "../operator.hpp"
#include "core/dates_fwd.hpp"
#include "core/jagged_2d_array.hpp"
#include <memory>
#include <string>
#include <Eigen/Core>

namespace averisera {

    template <class T> class Array2D;
    class CopulaGaussian;
    class Distribution;
    
    namespace microsim {

        class Contexts;
        class Person;
        template <class T> class Predicate;        

        /** Operator which implements the inheritance of one or more variables from the mother.
         */
        class OperatorInheritance: public Operator<Person> {
        public:
            /** Where in time the mother's distribution is taken from */
            class ReferenceDateType {
            public:
                virtual ~ReferenceDateType();

                /** Return the reference date, from which parent's data affect the child via inheritance */
                virtual Date reference_date(const Person& parent, const Person& child, const Contexts& ctx) const = 0;

                static const std::shared_ptr<const ReferenceDateType> CONCEPTION; /**< Return conception date */
                static const std::shared_ptr<const ReferenceDateType> BIRTH; /**< Return birth date */
            };          
            
            /**
              @param variables Names of inherited, correlated variables (size = N)
              @param predicate Operator predicate
              @param mother_distributions Distributions of N variables for the mother as of the date of birth of the child. Each row corresponds
              to a simulation date.
              @param child_distributions Distributions of N variables for the child at the age when we set the variables. Each row corresponds
              to a simulation date.
              @param copulas Copulas linking mother and child distributions, with dim = 2 * N. First N copula factors are mother
              variables, followed by N child variables. Each copula corresponds to a simulation date.
              @param date_offset Index of the first simulation date when the operator is going to be applied.
              @param ref_date_type Reference date type
              @param history_factories Vector of history factories
              @throws std::domain_error If dimensions do not match or a pointer is null
             */
            OperatorInheritance(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<Person>> predicate,
                                const Array2D<std::shared_ptr<const Distribution>>& mother_distributions,
                                const Array2D<std::shared_ptr<const Distribution>>& child_distributions,
                                const std::vector<std::shared_ptr<const CopulaGaussian>>& copulas,
                                unsigned int date_offset, std::shared_ptr<const ReferenceDateType> ref_date_type,
                                const std::vector<HistoryFactory::factory_t>& history_factories);
            
            void apply(const std::vector<std::shared_ptr<Person>>& selected, const Contexts& contexts) const override;

            const Predicate<Person>& predicate() const override {
                return *_predicate;
            }

			const HistoryGenerator<Person>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const std::string& name() const override {
				static const std::string str("Inheritance");
				return str;
			}
        private:
			HistoryGeneratorSimple<Person> hist_gen_;
            std::vector<std::string> _variables;
            std::shared_ptr<const Predicate<Person>> _predicate;
            Jagged2DArray<std::shared_ptr<const Distribution>> _distributions;
            std::vector<std::shared_ptr<const CopulaGaussian>> _copulas;
            unsigned int _date_offset;
            std::shared_ptr<const ReferenceDateType> _rdt;
            Eigen::VectorXd _zero;
        };
    }
}

#endif // __AVERISERA_MS_OPERATOR_INHERITANCE_H
