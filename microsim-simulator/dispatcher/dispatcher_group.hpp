#ifndef __AVERISERA_MS_DISPATCHER_GROUP_H
#define __AVERISERA_MS_DISPATCHER_GROUP_H

#include "../dispatcher.hpp"
#include "../predicate_factory.hpp"

namespace averisera {
    namespace microsim {
        template <class A> class Predicate;

        /**
        Dispatches argument to the first value whose predicate selects it.
        */
        template <class A, class H> class DispatcherGroup : public Dispatcher<A, H> {
        public:
            /** @param[in] predicates Vector of value predicates. All pointers must be non-null. Non-zero size. Predicates are referenced from first to last, the first one which
            matches is used to choose the returned value.
            @param[in] values Equal size to predicates.
            @param[in] requires Vector of required features.
            * @param[in] group_predicate Optional Predicate for the whole operator group. It should be functionally equivalent to logical sum of the predicates of member operators.            
            * @throw std::domain_error If predicates or values is empty, any pointer in predicates is null or predicates.size() != values.size()
            */
            DispatcherGroup(const std::vector<std::shared_ptr<const Predicate<A>>>& predicates, const std::vector<H>& values, const FeatureUser<Feature>::feature_set_t& requires, std::shared_ptr<const Predicate<A>> group_predicate = nullptr);

            H dispatch(const A& obj, const Contexts& contexts) const override;

            H dispatch_out_of_context(const A& obj) const override;

            const FeatureUser<Feature>::feature_set_t& requires() const override {
                return _requires;
            }

            std::shared_ptr<const Predicate<A> > predicate() const override {
                return _grp_pred;
            }
        private:
            std::vector<std::shared_ptr<const Predicate<A>>> _predicates;
            std::vector<H> _values;
            FeatureUser<Feature>::feature_set_t _requires;
            std::shared_ptr<const Predicate<A>> _grp_pred;
            unsigned int _size;
        };

        template <class A, class H> DispatcherGroup<A, H>::DispatcherGroup(const std::vector<std::shared_ptr<const Predicate<A>>>& predicates, const std::vector<H>& values, const FeatureUser<Feature>::feature_set_t& requires, std::shared_ptr<const Predicate<A>> group_predicate)
            : _predicates(predicates), _values(values), _requires(requires), _grp_pred(group_predicate), _size(static_cast<unsigned int>(values.size())) {
            if (predicates.size() != values.size()) {
                throw std::domain_error("DispatcherGroup: predicates and values have different sizes");
            }
            if (!_grp_pred) {
                _grp_pred = PredicateFactory::make_or(_predicates);
            } else {
                for (auto ptr : _predicates) {
                    if (!ptr) {
                        throw std::domain_error("DispatcherGroup: null predicate");
                    }
                }
            }
            if (values.empty()) {
                throw std::domain_error("DispatcherGroup: no values");
            }            
        }

        template <class A, class H> H DispatcherGroup<A, H>::dispatch(const A& obj, const Contexts& contexts) const {
            for (unsigned int i = 0; i < _size; ++i) {
                if (_predicates[i]->select(obj, contexts)) {
                    return _values[i];
                }
            }
            throw std::domain_error("DispatcherGroup: cannot handle argument");
        }

        template <class A, class H> H DispatcherGroup<A, H>::dispatch_out_of_context(const A& obj) const {
            for (unsigned int i = 0; i < _size; ++i) {
                if (_predicates[i]->select_out_of_context(obj)) {
                    return _values[i];
                }
            }
            throw std::domain_error("DispatcherGroup: cannot handle argument");
        }
    }
}

#endif // __AVERISERA_MS_DISPATCHER_GROUP_H
