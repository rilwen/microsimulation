// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_OPERATOR_INCREMENTER_MULTI_H
#define __AVERISERA_MS_OPERATOR_INCREMENTER_MULTI_H

#include "../operator_individual.hpp"
#include "../history_user_simple.hpp"
//#include <type_traits>

namespace averisera {
    class MultivariateDistribution;
    
    namespace microsim {
        class Actor;
        
        /** OperatorIndividual which increments the variable by drawing the delta from a distribution.
         * Non-instantaneous.
		 * @tparam T derived from Actor
         */
        template <class T> class OperatorIncrementerMulti: public OperatorIndividual<T> {
            //static_assert(std::is_base_of<Actor, T>::value, "T must be derived from Actor");
        public:
            /** 
              @param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule.
              @throw std::domain_error If predicate or any of the distributions is null. If variable is empty. If variables.size() != distributions[i].dim() for any i, or if variables.size() != history_factories.size(). If any history factory is null.
             */
            OperatorIncrementerMulti(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<T>> predicate, const std::vector<std::shared_ptr<const MultivariateDistribution>>& distributions, std::unique_ptr<Schedule>&& schedule);
            
            const Predicate<T>& predicate() const override {
                return *_predicate;
            }

            /** Dimension of the increment distributions */
            unsigned int dim() const {
                return static_cast<unsigned int>(_variables.size());
            }
            
            /** @throw std::out_of_range If not enough distributions */
            void apply(const std::shared_ptr<T>& obj, const Contexts& contexts) const override;

            bool active(Date date) const override {
                return Operator<T>::active(_schedule, date);
            }

			const typename HistoryUser<T>::use_reqvec_t& user_requirements() const override {
				return history_user_.user_requirements();
			}

			const std::string& name() const override {
				static const std::string str("IncrementerMulti");
				return str;
			}
        private:
			HistoryUserSimple<T> history_user_;
            std::vector<std::string> _variables;
            std::shared_ptr<const Predicate<T>> _predicate;            
            std::vector<std::shared_ptr<const MultivariateDistribution>> _distributions;
            std::unique_ptr<Schedule> _schedule;
        };
    }
}

#endif // __AVERISERA_MS_OPERATOR_INCREMENTER_H
