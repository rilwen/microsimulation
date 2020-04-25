#ifndef __AVERISERA_MICROSIM_DISPATCHER_FACTORY_H
#define __AVERISERA_MICROSIM_DISPATCHER_FACTORY_H

#include "dispatcher/dispatcher_operator_group.hpp"
#include "dispatcher/dispatcher_range_1d.hpp"
#include "dispatcher/dispatcher_constant.hpp"
#include "dispatcher/dispatcher_group.hpp"
#include "predicate/pred_true.hpp"

namespace averisera {
    namespace microsim {
        /** Factory methods for Dispatcher implementations. */
        namespace DispatcherFactory {
            /** @see DispatcherOperatorGroup */
            template <class T> std::unique_ptr<const Dispatcher<T, unsigned int>> make_operator_group(const std::vector<std::shared_ptr<const Operator<T>>>& operators, std::shared_ptr<const Predicate<T>> group_predicate = nullptr) {
                return std::unique_ptr<const Dispatcher<T, unsigned int>>(new DispatcherOperatorGroup<T>(operators, group_predicate));
            }
            
            /** @see DispatcherRange1D */
            template <class T> std::unique_ptr<const Dispatcher<T, unsigned int>> make_range_1d(std::shared_ptr<const typename DispatcherRange1D<T>::functor_t> functor, const std::vector<double>& thresholds) {
                return std::unique_ptr<const Dispatcher<T, unsigned int>>(new DispatcherRange1D<T>(functor, thresholds));
            }
            
            /** @see DispatcherConstant */
            template <class A, class H> std::unique_ptr<const Dispatcher<A, H>> make_constant(H default_value, std::shared_ptr<const Predicate<A>> predicate) {
                return std::unique_ptr<const Dispatcher<A, H>>(new DispatcherConstant<A, H>(default_value, predicate));
            }

            /** Make a DispatcherConstant which always applies */
            template <class A, class H> std::unique_ptr<const Dispatcher<A, H>> make_constant(H default_value) {
                return std::unique_ptr<const Dispatcher<A, H>>(new DispatcherConstant<A, H>(default_value, std::make_shared<PredTrue<A>>()));
            }

            /** @see DispatcherGroup */
            template <class A, class H> std::unique_ptr<const Dispatcher<A, H>> make_group(const std::vector<std::shared_ptr<const Predicate<A>>>& predicates, const std::vector<H>& values, const FeatureUser<Feature>::feature_set_t& requires, std::shared_ptr<const Predicate<A>> group_predicate = nullptr) {
                return std::unique_ptr<const Dispatcher<A, H>>(new DispatcherGroup<A, H>(predicates, values, requires, group_predicate));
            }
        }
    }
}

#endif // __AVERISERA_MICROSIM_DISPATCHER_FACTORY_H
