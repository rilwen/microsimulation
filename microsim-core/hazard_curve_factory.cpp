#include "hazard_curve_factory.hpp"
#include "core/log.hpp"
#include "core/stl_utils.hpp"
#include "core/time_series.hpp"
#include "hazard_curve/hazard_curve_flat.hpp"
#include "hazard_curve/hazard_curve_piecewise_constant.hpp"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        std::unique_ptr<HazardCurve> HazardCurveFactory::make_flat(double rate) {
            return std::unique_ptr<HazardCurve>(new HazardCurveFlat(rate));
        }
	    
        std::unique_ptr<HazardCurve> HazardCurveFactory::make_piecewise_constant(const TimeSeries<double, double>& rates) {
            return std::unique_ptr<HazardCurve>(new HazardCurvePiecewiseConstant(rates));
        }

        HazardCurveFactory::~HazardCurveFactory() {
        }

        class PiecewiseConstant: public HazardCurveFactory {
        public:
            std::unique_ptr<HazardCurve> build(const std::vector<double>& times, const std::vector<double>& jump_probs, bool conditional) const override {
				//TRACE() << "HazardCurveFactory::PiecewiseConstant::build: times=" << times << ", jump_probs=" << jump_probs << ", conditional=" << conditional;
				const size_t n = times.size();
				if (n != jump_probs.size()) {
                    throw std::domain_error("PiecewiseConstant: vector size mismatch");
                }
                if (times.empty()) {
                    throw std::domain_error("PiecewiseConstant: no data");
                }				
                std::vector<double> start_times(n);
				std::vector<double> rates;
                start_times[0] = 0;
				rates.reserve(n);
                std::copy(times.begin(), times.end() - 1, start_times.begin() + 1);                
                double prev_integrated_rate = 0;
				auto times_it = times.begin();
				auto start_times_it = start_times.begin();
                for (const double& jp: jump_probs) {
					//TRACE() << "HazardCurveFactory::PiecewiseConstant::build: jp=" << jp;
                    const double integrated_rate = - std::log1p(- jp);
					//TRACE() << "HazardCurveFactory::PiecewiseConstant::build: integrated_rate=" << integrated_rate;
                    const double delta_ihr = conditional ? integrated_rate : (integrated_rate - prev_integrated_rate);
					const double dt = *times_it - *start_times_it;
					//TRACE() << "HazardCurveFactory::PiecewiseConstant::build: dt=" << dt;
                    const double rate = delta_ihr / dt;
					rates.push_back(rate);
                    prev_integrated_rate = integrated_rate;
					++times_it; ++start_times_it;
                }
                TimeSeries<double, double> rates_ts(std::move(start_times), std::move(rates));
                return std::unique_ptr<HazardCurve>(new HazardCurvePiecewiseConstant(std::move(rates_ts)));
            }
        };

        const std::shared_ptr<const HazardCurveFactory> HazardCurveFactory::PIECEWISE_CONSTANT() {
            static const auto factory = std::make_shared<PiecewiseConstant>();
            return factory;
        }
    }
}
