// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_DISPATCHER_OPERATOR_GROUP_H
#define __AVERISERA_MS_DISPATCHER_OPERATOR_GROUP_H

#include "../dispatcher.hpp"
#include "../operator.hpp"
#include "../operator_group.hpp"
#include "../predicate.hpp"
#include "../predicate_factory.hpp"
#include "core/utils.hpp"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <vector>

namespace averisera {
    namespace microsim {
        /** Dispatches argument to the first operator which selects it chosen from a group of operators O_i with mutually exclusive predicates P_i: for given argument x, only one P_i(x) is true.
         */
        template <class T> class DispatcherOperatorGroup: public Dispatcher<T, unsigned int> {
        public:
            /** @param[in] operators Vector of operators. All pointers must be non-null. Non-zero size. All operators have to provide and require the
             * same features, and have the same value of is_instantaneous().
             * @param[in] group_predicate Optional Predicate for the whole operator group. It should be functionally equivalent to logical sum of the predicates of member operators. If it is null, we create a predicate by cloning the operators' predicates and combining them into a logical sum.
             * @throw std::domain_error If operators is empty, any pointer in it is empty or operators have different values of provides(),
             * requires() or is_instantaneous().
             */
            DispatcherOperatorGroup(const std::vector<std::shared_ptr<const Operator<T>>>& operators, std::shared_ptr<const Predicate<T>> group_predicate = nullptr)
            : _operators(operators), _grp_pred(group_predicate), _size(static_cast<unsigned int>(operators.size())) {
                OperatorGroup<T>::validate(_operators);
                if (!_grp_pred) {
                    std::vector<std::shared_ptr<const Predicate<T> > > predicates(_operators.size());
                    std::transform(_operators.begin(), _operators.end(), predicates.begin(), [](const std::shared_ptr<const Operator<T>>& ptr) { return std::shared_ptr<const Predicate<T> >(ptr->predicate().clone()); });
                    _grp_pred = PredicateFactory::make_or(std::move(predicates));
                }
            }
            
            unsigned int dispatch(const T& obj, const Contexts& contexts) const override {                
                for (unsigned int i = 0; i < _size; ++i) {
                    if (_operators[i]->predicate().select(obj, contexts)) {
                        return i;
                    }
                }
                throw std::domain_error("DispatcherOperatorGroup: cannot handle argument");
            }

            unsigned int dispatch_out_of_context(const T& obj) const override {                
                for (unsigned int i = 0; i < _size; ++i) {
                    if (_operators[i]->predicate().select_out_of_context(obj)) {
                        return i;
                    }
                }
                throw std::domain_error("DispatcherOperatorGroup: cannot handle argument");
            }
            
            const FeatureUser<Feature>::feature_set_t& requires() const {
                return _operators.front()->requires();
            }

            std::shared_ptr<const Predicate<T> > predicate() const override {
                return _grp_pred;
            }
        private:            
            std::vector<std::shared_ptr<const Operator<T>>> _operators;            
            std::shared_ptr<const Predicate<T>> _grp_pred;
            unsigned int _size;
        };
    }
}

#endif // __AVERISERA_MS_DISPATCHER_OPERATOR_GROUP_H
