// (C) Averisera Ltd 2014-2020
#include "relative_risk_value.hpp"
#include <cmath>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        RelativeRiskValue::RelativeRiskValue(double rr, Date rs, Date re, RelativeRiskValue::Type rt)
            : relative_risk(rr), ref_start(rs), ref_end(re), type(rt) {
            if (rr < 0) {
                throw std::domain_error("RelativeRiskValue: negative value");
            }
            if (std::isinf(rr)) {
                throw std::domain_error("RelativeRiskValue: infinite value");
            }
            if (std::isnan(rr)) {
                throw std::domain_error("RelativeRiskValue: NaN value");
            }
            if (re < rs) {
                throw std::domain_error("RelativeRiskValue: dates out of order");
            }
        }
    }
}
