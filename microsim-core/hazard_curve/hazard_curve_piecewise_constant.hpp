/*
* (C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_HAZARD_CURVE_PIECEWISE_CONSTANT_H
#define __AVERISERA_MS_HAZARD_CURVE_PIECEWISE_CONSTANT_H

#include "../hazard_curve.hpp"
#include "core/time_series.hpp"
#include <cassert>
#include <stdexcept>

namespace averisera {
	namespace microsim {
		/*! \brief Hazard curve with piecewise-constant rate. Left-continuous.
		*/
		class HazardCurvePiecewiseConstant: public HazardCurve {
		public:
            typedef size_t index_t;
            
			/*! Construct from a TimeSeries with pairs (h_i, t_i) where t_i = min_t h(t) = h_i.
			\param[in] rates TimeSeries with rates.
			\throw std::domain_error If first time is not 0 or time series is empty, or any of the hazard rates is negative.
			*/
			HazardCurvePiecewiseConstant(const TimeSeries<double, double>& rates);

			/*! Move-construct from a TimeSeries with pairs (h_i, t_i) where t_i = min_t h(t) = h_i.
			\param[in] rates TimeSeries with rates.
			\throw std::domain_error If first time is not 0 or time series is empty.
			*/
			HazardCurvePiecewiseConstant(TimeSeries<double, double>&& rates);

			double instantaneous_hazard_rate(double t) const;

			double average_hazard_rate(double t1, double t2) const;

			double integrated_hazard_rate(double t1, double t2) const;

			std::unique_ptr<HazardCurve> slide(double delta) const override;

			std::unique_ptr<HazardCurve> multiply_hazard_rate(double t0, double t1, double factor) const override;

			/*! Number of hazard rates defined. */
			index_t size() const {
				return _rates.size();
			}

			/*! Return idx-th hazard rate. */
			double rate(index_t idx) const {
				assert(idx < size());
				return _rates[idx].second;
			}

			/*! Set the idx-th hazard rate. 
            \param idx < size()
            \param rate >= 0
            \throw std::domain_error If rate < 0
            */
			void set_rate(index_t idx, double rate) {
				assert(idx < size());
                if (rate < 0) {
                    throw std::domain_error("HazardCurvePiecewiseConstant: negative hazard rate");
                }
                _rates.value_at_index(idx) = rate;                
                recalc_ihr(idx);
			}

            double calc_t2(double t1, double ihr) const override;
		private:
			using HazardCurve::validate;
			void validate() const;
            
            /*! \param[in] from Index of the first rate to be integrated */
            void recalc_ihr(index_t from);
            void construct();

			TimeSeries<double, double> _rates;
            TimeSeries<double, double> _ihr; /*!< _ihr(t) = \int_0^t h(s) ds */
		};
	}
}

#endif // __AVERISERA_MS_HAZARD_CURVE_PIECEWISE_CONSTANT_H
