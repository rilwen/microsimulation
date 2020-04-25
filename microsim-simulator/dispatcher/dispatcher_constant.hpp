#ifndef __AVERISERA_MS_DISPATCHER_CONSTANT_HPP
#define __AVERISERA_MS_DISPATCHER_CONSTANT_HPP

#include "../dispatcher.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        /** Dispatcher which always returns the default value */
        template <class A, class H> class DispatcherConstant: public Dispatcher<A, H> {
        public:
            /**
             * @param[in] dflt Default value to return
             * @param[in] pred Predicate
             * @throw std::domain_error If pred is null
             */
            DispatcherConstant(H dflt, std::shared_ptr<const Predicate<A>> pred)
            : _dflt(dflt), _pred(pred) {
                if (!pred) {
                    throw std::domain_error("DispatcherConstant: null predicate");
                }
            }
            
            H dispatch(const A& obj, const Contexts& contexts) const override {
                return _dflt;
            }

            H dispatch_out_of_context(const A& obj) const override {
                return _dflt;
            }
            
            const FeatureUser<Feature>::feature_set_t& requires() const {
                return Feature::empty();
            }

            std::shared_ptr<const Predicate<A> > predicate() const override {
                return _pred;
            }

			/** For testing */
			const H& value() const {
				return _dflt;
			}
        private:
            H _dflt;
            std::shared_ptr<const Predicate<A>> _pred;
        };
    }
}

#endif // __AVERISERA_MS_DISPATCHER_CONSTANT_HPP
