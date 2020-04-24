#include "hazard_curve_flat.hpp"
#include "hazard_curve_piecewise_constant.hpp"

namespace averisera {
	namespace microsim {
		std::unique_ptr<HazardCurve> HazardCurveFlat::multiply_hazard_rate(const double t0, const double t1, const double factor) const {
			validate(t0, t1, factor);
			if (t0 != t1 && factor != 1) {
				if (t0 > 0) {
					std::vector<TimeSeries<double, double>::tvpair_t> data(3);
					data[0] = { 0.0, _rate };
					data[1] = { t0, factor * _rate };
					data[2] = { t1, _rate };
					return std::make_unique<HazardCurvePiecewiseConstant>(TimeSeries<double, double>(std::move(data)));
				} else {
					if (std::isfinite(t1)) {
						std::vector<TimeSeries<double, double>::tvpair_t> data(2);
						data[0] = { 0.0, factor * _rate };
						data[1] = { t1, _rate };
						return std::make_unique<HazardCurvePiecewiseConstant>(TimeSeries<double, double>(std::move(data)));
					} else {
						return std::make_unique<HazardCurveFlat>(factor * _rate);
					}
				}
			} else {
				return clone();
			}
		}
	}
}
