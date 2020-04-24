/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_HAZARD_CURVE_H
#define __AVERISERA_MS_HAZARD_CURVE_H

#include "core/log.hpp"
#include <memory>

namespace averisera {
    namespace microsim {
        /*! \brief Abstract hazard curve.
         * 
         *  Describes a single jump process happening in continuous time.
         * 
         */
        class HazardCurve {
        public:
            virtual ~HazardCurve();
                        
            /*! \brief Instantaneous hazard rate.
             * 
             * Return the hazard rate at time t
             * \param[in] t time, t >= 0
             * \return Hazard rate >= 0
             */
            virtual double instantaneous_hazard_rate(double t) const = 0;
            
            /*! \brief Average hazard rate between two times.
             * 
             * Return the average hazard rate
             * \f$ \bar{h} = (t_2 - t_1)^{-1} \int_{t_1}^{t_2} h(s) ds \f$
             * between times t1 and t2.
             * If t1 == t2, return instantaneous hazard rate for t1.
             * 
             * \param[in] t1 Start time of the averaging period, t1 >= 0
             * \param[in] t2 End time of the averaging period_t, t2 >= t1
             \return Value >= 0
             */
            virtual double average_hazard_rate(double t1, double t2) const = 0;
            
            /*! \brief Integrated hazard rate between two times.
             * 
             * Return the integrated hazard rate
             * \f$ H = \int_{t_1}^{t_2} h(s) ds \f$
             * between times t1 and t2.
             * If t1 == t2, return 0.
             * 
             * \param[in] t1 Start time of the integration period, t1 >= 0
             * \param[in] t2 End time of the integration period, t2 >= t1
             * \return Value >= 0
             */
            virtual double integrated_hazard_rate(double t1, double t2) const = 0;
            
            /*! \brief Slide along the hazard curve, producing a new one.
             * 
             * Return a new hazard curve described by the formula \f$ h'(s) = h(t + \Delta) \f$
             * 
             * \param[in] delta Non-negative time shift.
             \throw std::out_of_range If delta < 0
             */
            virtual std::unique_ptr<HazardCurve> slide(double delta) const = 0;

            /*! Find such t2, that ihr == integrated_hazard_rate(t1, t2)
            \param t1 >= 0
            \param ihr >= 0
            \return t2 >= t1. If ihr == 0, return t1.
            */
            virtual double calc_t2(double t1, double ihr) const = 0;

            /*! Conditional or unconditional jump probability 
              \param integrated_hazard_rate >= 0
             */
            static double jump_probability(double integrated_hazard_rate);

			double jump_probability(double t0, double t1) const {
				return jump_probability(integrated_hazard_rate(t0, t1));
			}

            /*! Inverse function to jump_probability() 
              \param p in [0, 1]
             */
            static double integrated_hazard_rate_from_jump_proba(double p);

			/*! Extrapolate jump probability to a period x times longer */
			static double extrapolate_proba(double p, double x) {
				//TRACE() << "HazardCurve::extrapolate_proba: p=" << p << ", x=" << x;
				if (x != 1 && p != 0) {
					return jump_probability(x * integrated_hazard_rate_from_jump_proba(p));
				} else {
					return p;
				}
			}

			/*! Create a copy */
			std::unique_ptr<HazardCurve> clone() const {
				return slide(0.0);
			}

			/*! Return a HazardCurve with hazard rate h(s) -> factor * h(s) for s in [t0, t1) 
			\throw std::out_of_range If factor < 0, t0 > t1 or t0 < 0
			*/
			virtual std::unique_ptr<HazardCurve> multiply_hazard_rate(double t0, double t1, double factor) const = 0;

			/*! Return a HazardCurve c with hazard rate rescaled so that c->jump_probability(t0, t1) = factor * this->jump_probability(t0, t1).
			\throw std::out_of_range If factor * this->jump_probability(t0, t1) > 1, factor < 0, t0 > t1 or t0 < 0.
			*/
			std::unique_ptr<HazardCurve> multiply_probability(double t0, double t1, double factor) const;
		protected:
			static void validate(double t0, double t1, double factor);
        };
    }
}

#endif // __AVERISERA_MS_HAZARD_CURVE_H
