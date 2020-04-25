#ifndef __AVERISERA_MS_RELATIVE_RISK_RANGE_1D_H
#define __AVERISERA_MS_RELATIVE_RISK_RANGE_1D_H

#include "../relative_risk.hpp"
#include "../dispatcher/dispatcher_range_1d.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        /** Relative risk calculated based on the range of another property */
        template <class A> class RelativeRiskRange1D : public RelativeRisk<A> {
        public:
            typedef Functor<A, double> functor_t;

            /**
            @param[in] functor Maps arguments to values over the range
            @param[in] thresholds Vector of thresholds between ranges.
            @param[in] values Relative risk values for each range
            @throw std::domain_error If functor is null. If thresholds are not strictly increasing. If values.size() != thresholds.size() + 1. If any rel. risk value is negative.
            */
            RelativeRiskRange1D(std::shared_ptr<const functor_t> functor, const std::vector<double>& thresholds, const std::vector<RelativeRiskValueUnbound>& values);

            RelativeRiskValueUnbound calc_relative_risk_unbound(const A& arg, const Contexts& ctx) const override;

            const FeatureUser<Feature>::feature_set_t& requires() const override {
                return _dispatcher.requires();
            }
        private:
            DispatcherRange1D<A> _dispatcher;
            std::vector<RelativeRiskValueUnbound> _values;
        };

        template <class A> RelativeRiskRange1D<A>::RelativeRiskRange1D(std::shared_ptr<const functor_t> functor, const std::vector<double>& thresholds, const std::vector<RelativeRiskValueUnbound>& values)
            : _dispatcher(functor, thresholds), _values(values) {
            if (values.size() != thresholds.size() + 1) {
                throw std::domain_error("RelativeRiskRange1D: bad values size");
            }
            if (std::any_of(values.begin(), values.end(), [](double x) { return x < 0; })) {
                throw std::domain_error("RelativeRiskRange1D: negative relative risks");
            }
        }

        template <class A> RelativeRiskValueUnbound RelativeRiskRange1D<A>::calc_relative_risk_unbound(const A& arg, const Contexts& ctx) const {
            const unsigned int idx = _dispatcher.dispatch(arg, ctx);
            assert(idx < _values.size());
            return _values[idx];
        }
    }
}

#endif // __AVERISERA_MS_RELATIVE_RISK_RANGE_1D_H
