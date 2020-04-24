#ifndef __AVERISERA_MS_RELATIVE_RISK_H
#define __AVERISERA_MS_RELATIVE_RISK_H

#include "functor.hpp"
#include "core/dates.hpp"
#include "core/period.hpp"
#include "contexts.hpp"
#include "mutable_context.hpp"
#include "immutable_context.hpp"
#include "microsim-core/schedule.hpp"
#include "microsim-core/relative_risk_value.hpp"

namespace averisera {
    namespace microsim {
        /*! Way to encoude RelativeRiskValue with Type == SCALABLE without using dates */
        struct RelativeRiskValueUnbound {
            /*! Create default value with 1.0 value */
            RelativeRiskValueUnbound();

            /*
              \throw std::domain_error If value < 0 or period <= 0
            */
            RelativeRiskValueUnbound(double v, Period p);

            bool operator==(const RelativeRiskValueUnbound& other) const;

            double relative_risk;
            Period period;
        };
	
        /*! Relative risk of some event */       
        template <class A> class RelativeRisk: public Functor<A, RelativeRiskValue> {
        public:
            /*! Returns scalable RelativeRiskValue by default */
            RelativeRiskValue operator()(const A& arg, const Contexts& ctx) const override {
                const Date begin = ctx.asof();
                const RelativeRiskValueUnbound ru = calc_relative_risk_unbound(arg, ctx);
                const Date end = begin + ru.period;
                return RelativeRiskValue(ru.relative_risk, begin, end, RelativeRiskValue::Type::SCALABLE);
            }

            virtual RelativeRiskValueUnbound calc_relative_risk_unbound(const A& arg, const Contexts& ctx) const = 0;
        };
    }
}

#endif // __AVERISERA_MS_RELATIVE_RISK_H
