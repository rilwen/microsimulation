// (C) Averisera Ltd 2014-2020
#include "relative_risk.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        

        RelativeRiskValueUnbound::RelativeRiskValueUnbound()
            : RelativeRiskValueUnbound(1., Period(PeriodType::DAYS, 1))
        {}

        RelativeRiskValueUnbound::RelativeRiskValueUnbound(double v, Period p)
            : relative_risk(v), period(p) {
            if (v < 0) {
                throw std::domain_error("RelativeRiskValueUnbound: negative value");
            }
            if (p.days() <= 0) {
                throw std::domain_error("RelativeRiskValueUnbound: negative or zero period");
            }
        }

        bool RelativeRiskValueUnbound::operator==(const RelativeRiskValueUnbound& other) const {
            return relative_risk == other.relative_risk && period == other.period;
        }
    }
}
