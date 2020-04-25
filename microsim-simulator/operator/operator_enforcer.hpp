#ifndef __AVERISERA_OPERATOR_ENFORCER_H
#define __AVERISERA_OPERATOR_ENFORCER_H

#include "../history_generator_simple.hpp"
#include "../operator.hpp"
#include "microsim-core/schedule.hpp"
#include <memory>
//#include <type_traits>

namespace averisera {
    class Distribution;
    
    namespace microsim {        
        class Actor;
        
        /** Enforces a distribution of a variable 
         * 
         * @tparam T Derived from Actor
         */
        template <class T> class OperatorEnforcer: public Operator<T> {
            //static_assert(std::is_base_of<Actor, T>::value, "T must be derived from Actor");
        public:
            /** 
              @param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule.
              @throw std::domain_error If predicate or any of the distributions is null. If variable is empty.
              @throw std::out_of_range If schedule is not null and there is less distributions than dates in the schedule.
             */
            OperatorEnforcer(const std::string& variable,
                             std::shared_ptr<const Predicate<T>> predicate,
                             const std::vector<std::shared_ptr<const Distribution>>& distributions,
                             HistoryFactory::factory_t history_factory,
                             std::unique_ptr<Schedule>&& schedule);
            
            const Predicate<T>& predicate() const override {
                return *_predicate;
            }
    
            /** @throw std::out_of_range If not enough distributions.
             @throw std::logic_error If custom schedule does not fit in context schedule */            
            void apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const override;

            bool active(Date date) const override {
				// operator is active on date if the date is in the schedule or schedule is null
                return Operator<T>::active(_schedule, date);
            }

			const typename HistoryGenerator<T>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const std::string& name() const override {
				static const std::string str("Enforcer");
				return str;
			}
        private:
			HistoryGeneratorSimple<T> hist_gen_;
            std::string _variable;
            std::shared_ptr<const Predicate<T>> _predicate;            
            std::vector<std::shared_ptr<const Distribution>> _distributions;
            std::unique_ptr<Schedule> _schedule;
        };
        
        
    }
}

#endif // __AVERISERA_OPERATOR_ENFORCER_H
