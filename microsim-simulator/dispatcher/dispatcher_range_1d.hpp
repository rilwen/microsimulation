#ifndef __AVERISERA_MS_DISPATCHER_RANGE_1D_H
#define __AVERISERA_MS_DISPATCHER_RANGE_1D_H

#include "../dispatcher.hpp"
#include "../functor.hpp"
#include "../predicate_factory.hpp"
#include <algorithm>
#include <cassert>
#include <vector>

namespace averisera {
    namespace microsim {
        /** For each argument calculate a real quantity and map to the appropriate range */
        template <class T> class DispatcherRange1D: public Dispatcher<T, unsigned int> {
        public:
            typedef double range_value_t;
            typedef Functor<T, range_value_t> functor_t;
            
            /**
             * @param[in] functor Maps arguments to values over the range
             * @param[in] thresholds Vector of thresholds between ranges.
             * @throw std::domain_error If functor is null. If thresholds are not strictly increasing.        
             */
            DispatcherRange1D(std::shared_ptr<const functor_t> functor, const std::vector<double>& thresholds)
                : _functor(functor), _thresholds(thresholds),
                  _pred(PredicateFactory::make_true<T>()) {
                assert(_pred);
                if (!_functor) {
                    throw std::domain_error("DispatcherRange1D: null functor");
                }
                if (!std::is_sorted(_thresholds.begin(), _thresholds.end())) {
                    throw std::domain_error("DispatcherRange1D: thresholds are not sorted");
                }
                if (std::adjacent_find(_thresholds.begin(), _thresholds.end()) != _thresholds.end()) {
                    throw std::domain_error("DispatcherRange1D: not all thresholds unique");
                }
            }
            
            /** Return the index of the range, assuming that each range is [ thresholds[i-1], thresholds[i] ). */
            unsigned int dispatch(const T& obj, const Contexts& contexts) const override {
                const double x = (*_functor)(obj, contexts);
                const auto it = std::upper_bound(_thresholds.begin(), _thresholds.end(), x);
                return static_cast<unsigned int>(std::distance(_thresholds.begin(), it));
            }

            unsigned int dispatch_out_of_context(const T& obj) const override {
                return 0;
            }
            
            const FeatureUser<Feature>::feature_set_t& requires() const {
                return _functor->requires();
            }

            std::shared_ptr<const Predicate<T> > predicate() const override {
                return _pred;
            }
        private:
            std::shared_ptr<const functor_t> _functor;
            std::vector<double> _thresholds;
            std::shared_ptr<const Predicate<T> > _pred;
        };
    }
}

#endif // __AVERISERA_MS_DISPATCHER_RANGE_1D_H
