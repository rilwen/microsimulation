#ifndef __AVERISERA_MS_RELATIVE_RISK_FACTORY_H
#define __AVERISERA_MS_RELATIVE_RISK_FACTORY_H

#include "relative_risk/relative_risk_constant.hpp"
#include "relative_risk/relative_risk_range_1d.hpp"
#include "relative_risk/relative_risk_from_dispatcher.hpp"

namespace averisera {
    namespace microsim {
        namespace RelativeRiskFactory {
            /** @see RelativeRiskConstant */
            template <class A> std::unique_ptr<const RelativeRisk<A>> make_constant(RelativeRiskValueUnbound value) {
                return std::unique_ptr<const RelativeRisk<A>>(new RelativeRiskConstant<A>(value));
            }

            /** @see RelativeRiskRange1D */
            template <class A> std::unique_ptr<const RelativeRisk<A>> make_range_1d(std::shared_ptr<const typename RelativeRiskRange1D<A>::functor_t> functor, const std::vector<double>& thresholds, const std::vector<RelativeRiskValueUnbound>& values) {
                return std::unique_ptr<const RelativeRisk<A>>(new RelativeRiskRange1D<A>(functor, thresholds, values));
            }

            /** @see RelativeRiskFromDispatcher */
            template <class A> std::unique_ptr<const RelativeRisk<A>> make_from_dispatcher(std::shared_ptr<const Dispatcher<A, RelativeRiskValueUnbound>> dispatcher) {
                return std::unique_ptr<const RelativeRisk<A>>(new RelativeRiskFromDispatcher<A>(dispatcher));
            }
        }
    }
}

#endif // __AVERISERA_MS_RELATIVE_RISK_FACTORY_H