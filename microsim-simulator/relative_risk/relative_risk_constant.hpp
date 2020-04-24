#ifndef __AVERISERA_MS_RELATIVE_RISK_CONSTANT_H
#define __AVERISERA_MS_RELATIVE_RISK_CONSTANT_H

#include "../relative_risk.hpp"
#include "../feature.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        /*! Constant RelativeRisk */
        template <class A> class RelativeRiskConstant: public RelativeRisk<A> {
        public:
            /*!
            \throw std::out_of_range If value.relative_risk < 0 or value.period <= 0
            */
            RelativeRiskConstant(RelativeRiskValueUnbound value)
            : _value(value) {              
            }
            
            RelativeRiskValueUnbound calc_relative_risk_unbound(const A& arg, const Contexts& ctx) const override {
                return _value;
            }
            
            const FeatureUser<Feature>::feature_set_t& requires() const override {
                return Feature::empty();
            }
        private:
            RelativeRiskValueUnbound _value;
        };
    }
}

#endif // __AVERISERA_MS_RELATIVE_RISK_CONSTANT_H
