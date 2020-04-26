// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_RELATIVE_RISK_FROM_DISPATCHER_H
#define __AVERISERA_MS_RELATIVE_RISK_FROM_DISPATCHER_H

#include <memory>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        template <class A, class H> class Dispatcher;

        /** RelativeRisk calculated from Dispatcher */
        template <class A> class RelativeRiskFromDispatcher : public RelativeRisk<A> {
        public:
            /** @throw std::domain_error If dispatcher is null */
            RelativeRiskFromDispatcher(std::shared_ptr<const Dispatcher<A, RelativeRiskValueUnbound>> dispatcher)
                : _disp(dispatcher) {
                if (!dispatcher) {
                    throw std::domain_error("RelativeRiskFromDispatcher: dispatcher is null");
                }
            }

            RelativeRiskValueUnbound calc_relative_risk_unbound(const A& arg, const Contexts& ctx) const override {
                return _disp->dispatch(arg, ctx);
            }

            const FeatureUser<Feature>::feature_set_t& requires() const override {
                return _disp->requires();
            }
        private:
            std::shared_ptr<const Dispatcher<A, RelativeRiskValueUnbound>> _disp;
        };
    }
}

#endif // __AVERISERA_MS_RELATIVE_RISK_FROM_DISPATCHER_H
