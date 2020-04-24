#ifndef __AVERISERA_OPERATOR_ENFORCER_MULTI_H
#define __AVERISERA_OPERATOR_ENFORCER_MULTI_H

#include "../history_generator_simple.hpp"
#include "../operator.hpp"
//#include <type_traits>

namespace averisera {
    class MultivariateDistribution;
    
    namespace microsim {
        
        class Actor;
        
        /*! Enforces a multidimensional distribution of several variables
         * 
         * \tparam T Derived from Actor
         */
        template <class T> class OperatorEnforcerMulti: public Operator<T> {
            //static_assert(std::is_base_of<Actor, T>::value, "T must be derived from Actor");
        public:
            /*! 
              \param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule.
              \throw std::domain_error If predicate or any of the distributions is null. If variable is empty. If variables.size() != distributions[i].dim() for any i, or if variables.size() != history_factories.size(). If any history factory is null.
             */
            OperatorEnforcerMulti(const std::vector<std::string>& variables, std::shared_ptr<const Predicate<T>> predicate, const std::vector<std::shared_ptr<const MultivariateDistribution>>& distributions,
                                  const std::vector<HistoryFactory::factory_t>& history_factories, std::unique_ptr<Schedule>&& schedule);
            
            const Predicate<T>& predicate() const override {
                return *_predicate;
            }
            
            void apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const override;

            /*! Dimension of the enforced distributions */
            unsigned int dim() const {
                return static_cast<unsigned int>(_variables.size());
            }

            bool active(Date date) const override {
                return Operator<T>::active(_schedule, date);
            }

			const typename HistoryGenerator<T>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const std::string& name() const override {
				static const std::string str("EnforcerMulti");
				return str;
			}
        private:
			HistoryGeneratorSimple<T> hist_gen_;
            std::vector<std::string> _variables;
            std::shared_ptr<const Predicate<T>> _predicate;            
            std::vector<std::shared_ptr<const MultivariateDistribution>> _distributions;
            std::unique_ptr<Schedule> _schedule;
        };
        
        
    }
}

#endif // __AVERISERA_OPERATOR_ENFORCER_MULTI_H
