/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_HAZARD_CURVE_FLAT_H
#define __AVERISERA_MS_HAZARD_CURVE_FLAT_H

#include "../hazard_curve.hpp"
#include "core/log.hpp"
#include <cassert>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        /** @brief Hazard curve with constant hazard rate. */
        class HazardCurveFlat: public HazardCurve {
        public:
            /**
            @param rate >= 0
            */
            HazardCurveFlat(double rate)
            : _rate(rate)
            {
                assert(rate >= 0);
            }
            
            double instantaneous_hazard_rate(double) const {
				LOG_TRACE() << "HazardCurveFlat: ihr=" << _rate;
                return _rate;
            }
            
            double average_hazard_rate(double, double) const {
                return _rate;
            }
            
            double integrated_hazard_rate(double t1, double t2) const {
                assert(t1 >= 0);
                assert(t2 >= t1);
                return (t2 - t1) * _rate;
            }

			std::unique_ptr<HazardCurve> slide(double delta) const override {
				if (delta < 0) {
					throw std::out_of_range("HazardCurveFlat: negative delta");
				}
				return std::unique_ptr<HazardCurve>(new HazardCurveFlat(_rate));
			}

			std::unique_ptr<HazardCurve> multiply_hazard_rate(double t0, double t1, double factor) const override;
            
            double rate() const {
                return _rate;
            }
            
            /**
            @param rate >= 0
            */
            void set_rate(double rate) {
                assert(rate >= 0);
                _rate = rate;
            }

            double calc_t2(double t1, double ihr) const override {
                assert(t1 >= 0);
                assert(ihr >= 0);
                if (ihr > 0) {
                    return t1 + ihr / _rate;
                } else {
                    return t1;
                }
            }
        private:
            double _rate;
        };
    }
}

#endif //__AVERISERA_MS_HAZARD_CURVE_FLAT_H
