#ifndef __AVERISERA_MS_HAZARD_CURVE_FACTORY_H
#define __AVERISERA_MS_HAZARD_CURVE_FACTORY_H

#include <memory>
#include <vector>

namespace averisera {

    template <class T, class V> class TimeSeries;
    
    namespace microsim {
        class HazardCurve;
	
        /** Creates implementations of HazardCurve */
        class HazardCurveFactory {
        public:
            /** HazardCurve with constant rate */
            static std::unique_ptr<HazardCurve> make_flat(double rate);

            /** Construct from a TimeSeries with pairs (h_i, t_i) where t_i = min_t h(t) = h_i.
              @param[in] rates TimeSeries with rates.
              @throw std::domain_error If first time is not 0 or time series is empty.
            */
            static std::unique_ptr<HazardCurve> make_piecewise_constant(const TimeSeries<double, double>& rates);

            virtual ~HazardCurveFactory();

            /** Construct a HazardCurve given a vector of times t_i and probabilities p_i = P(tau < t_i) (if conditional == false) or p_i = P(tau < t_i | t_{i-1} <= tau ) (if conditional == true), where tau is jump time. 
              @param times Times t_i > 0 in strictly increasing order
              @param jump_probs Probabilities p_i \in [0, 1], non-decreasing
			  @param conditional Are jump probabilities conditional on surviving till previous time, or unconditional (from t=0)
              @throw std::domain_error If times.empty(), times.size() != jump_probs.size()
             */
            virtual std::unique_ptr<HazardCurve> build(const std::vector<double>& times, const std::vector<double>& jump_probs, bool conditional) const = 0;

            static const std::shared_ptr<const HazardCurveFactory> PIECEWISE_CONSTANT(); /**< Creates piecewise-constant hazard curves */
        };        
    }
}

#endif // __AVERISERA_MS_HAZARD_CURVE_FACTORY_H
