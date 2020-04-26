// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_OPERATOR_INCREMENTER_H
#define __AVERISERA_MS_OPERATOR_INCREMENTER_H

#include "../operator_individual.hpp"
#include "../history_factory.hpp"
#include "../history_user_simple.hpp"
#include "microsim-core/schedule.hpp"
//#include <type_traits>

namespace averisera {
    class Distribution;
    
    namespace microsim {
        /** OperatorIndividual which increments the variable by drawing the delta from a distribution.
         * Non-instantaneous.
		 * @tparam T derived from Actor
         */
        template <class T> class OperatorIncrementer: public OperatorIndividual<T> {
            //static_assert(std::is_base_of<Actor, T>::value, "T must be derived from Actor");
        public:
            /** @param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule. 
              @throw std::domain_error If predicate or any of the distributions is null. If variable is empty.
             */
            OperatorIncrementer(const std::string& variable, std::shared_ptr<const Predicate<T>> predicate, const std::vector<std::shared_ptr<const Distribution>>& distributions,
                                std::unique_ptr<Schedule>&& schedule);
            
            const Predicate<T>& predicate() const override {
                return *_predicate;
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
				static const std::string str("Incrementer");
				return str;
			}
        private:
			HistoryUserSimple<T> history_user_;
            std::string _variable;
            std::shared_ptr<const Predicate<T>> _predicate;            
            std::vector<std::shared_ptr<const Distribution>> _distributions;
            std::unique_ptr<Schedule> _schedule;            
        };
    }
}

#endif // __AVERISERA_MS_OPERATOR_INCREMENTER_H
