#include "anchored_hazard_curve.hpp"
#include "hazard_curve.hpp"
#include "hazard_curve_factory.hpp"
#include "relative_risk_value.hpp"
#include "core/daycount.hpp"
#include "core/log.hpp"
#include "core/period.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <stdexcept>
#include <boost/assert.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
    namespace microsim {
        AnchoredHazardCurve::AnchoredHazardCurve(Date start, std::shared_ptr<const Daycount> daycount, std::unique_ptr<const HazardCurve>&& hazard_curve)
            : _start(start), _daycount(daycount), _hazard_curve(std::move(hazard_curve)) {
            if (_start.is_special()) {
                throw std::domain_error("AnchoredHazardCurve:: start date is not valid");
            }
            if (!_daycount) {
                throw std::domain_error("AnchoredHazardCurve: daycount is null");                
            }
            if (!_hazard_curve) {
                throw std::domain_error("AnchoredHazardCurve: hazard curve is null");
            }
        }

        AnchoredHazardCurve::~AnchoredHazardCurve() {
        }

        double AnchoredHazardCurve::average_hazard_rate(Date d1, Date d2) const {
            if (d1 < _start || d2 < d1) {
                throw std::out_of_range("AnchoredHazardCurve: inputs out of range");
            }
            return _hazard_curve->average_hazard_rate(_daycount->calc(_start, d1), _daycount->calc(_start, d2));
        }

        double AnchoredHazardCurve::integrated_hazard_rate(Date d1, Date d2) const {
            if (d1 < _start || d2 < d1) {
                throw std::out_of_range("AnchoredHazardCurve: inputs out of range");
            }
            return _hazard_curve->integrated_hazard_rate(_daycount->calc(_start, d1), _daycount->calc(_start, d2));
        }

        double AnchoredHazardCurve::integrated_hazard_rate(Date d1, Date d2, const HazardRateMultiplier hazard_rate_multiplier) const {
            if (d1 < _start || d2 < d1) {
                throw std::out_of_range("AnchoredHazardCurve: inputs out of range");
            }
            return integrated_hazard_rate(d1, d2, &hazard_rate_multiplier, (&hazard_rate_multiplier) + 1);
        }

        // template <class C> static void sort(C& hazard_rate_multipliers) {
        //     std::sort(hazard_rate_multipliers.begin(), hazard_rate_multipliers.end(), [](const HazardRateMultiplier& m1, const HazardRateMultiplier& m2) {
        //             if (m1.from < m2.from) {
        //                 return true;
        //             } else if (m1.from > m2.from) {
        //                 return false;
        //             } else {
        //                 return m1.to < m2.to;
        //             }
        //         });
        // }

        template <class I> double AnchoredHazardCurve::integrated_hazard_rate(Date d1, Date d2, I begin, I end) const {
            assert(d1 <= d2);
            assert(d1 >= _start);
            if (begin == end) {
                return integrated_hazard_rate(d1, d2);
            } else {
                const HazardRateMultiplier& hrm = *begin;
                const I next = begin + 1;
                if (hrm.value != 1) {
                    if (hrm.to <= d1 || hrm.from >= d2) {
                        return integrated_hazard_rate(d1, d2, next, end);
                    }
                    const Date a = std::max(d1, hrm.from);
                    const Date b = std::min(d2, hrm.to);
                    assert(b >= a);
                    assert(a >= d1);
                    assert(b <= d2);
                    assert(a >= hrm.from);
                    assert(b <= hrm.to);
                    if (a < b) {
                        double result = hrm.value * integrated_hazard_rate(a, b, next, end);
                        if (a > d1) {
                            result += integrated_hazard_rate(d1, a, next, end);
                        }
                        if (b < d2) {
                            result += integrated_hazard_rate(b, d2, next, end);
                        }
                        return result;
                    } else {
                        return integrated_hazard_rate(d1, d2, next, end);
                    }
                } else {
                    return integrated_hazard_rate(d1, d2, next, end);
                }
            }
        }

        double AnchoredHazardCurve::integrated_hazard_rate(const Date d1, const Date d2, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const {
            if (d1 < _start || d2 < d1) {
                throw std::out_of_range("AnchoredHazardCurve: inputs out of range");
            }
            if (hazard_rate_multipliers.size() > 50) {
                // TODO: switch to other, non-recursive solution
                throw std::runtime_error("AnchoredHazardCurve: more than 50 multipliers unsupported");
            }
            return integrated_hazard_rate(d1, d2, hazard_rate_multipliers.begin(), hazard_rate_multipliers.end());
        }

        double AnchoredHazardCurve::jump_probability(Date d) const {
            if (d < _start) {
                throw std::out_of_range("AnchoredHazardCurve: inputs out of range");
            }
            const double ihr = _hazard_curve->integrated_hazard_rate(0, _daycount->calc(_start, d));
            return HazardCurve::jump_probability(ihr);
        }

		double AnchoredHazardCurve::conditional_jump_probability(Date d1, Date d2) const {
			const double ihr = integrated_hazard_rate(d1, d2);
			//TRACE() << "AnchoredHazardCurve::conditional_jump_probability: ihr(" << d1 << ", " << d2 << ")=" << ihr;
			return HazardCurve::jump_probability(ihr);
		}

        double AnchoredHazardCurve::conditional_jump_probability(Date d1, Date d2, const HazardRateMultiplier hazard_rate_multiplier) const {
            const double ihr = integrated_hazard_rate(d1, d2, hazard_rate_multiplier);
			return HazardCurve::jump_probability(ihr);
        }

        double AnchoredHazardCurve::conditional_jump_probability(Date d1, Date d2, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const {
            const double ihr = integrated_hazard_rate(d1, d2, hazard_rate_multipliers);
			return HazardCurve::jump_probability(ihr);
        }

        Date AnchoredHazardCurve::calc_next_date(Date d1, double ihr) const {
            if (d1 < _start || ihr < 0) {
                throw std::out_of_range("AnchoredHazardCurve: inputs out of range");
            }
            const double t1 = _daycount->calc(_start, d1);
            BOOST_ASSERT_MSG(t1 >= 0, boost::lexical_cast<std::string>(t1).c_str());
            const double t2 = _hazard_curve->calc_t2(t1, ihr);
            BOOST_ASSERT_MSG(t2 >= t1, boost::lexical_cast<std::string>(t2).c_str());
            const double dt = t2 - t1;
            BOOST_ASSERT_MSG(dt >= 0, boost::lexical_cast<std::string>(dt).c_str());
            if (std::isinf(dt)) {
                return Date::POS_INF;
            } else {
                try {
                    const Date result = _daycount->add_year_fraction(d1, dt);
                    BOOST_ASSERT_MSG(result.is_pos_infinity() || result >= d1, boost::str(boost::format("%s %s %s %g %g %g") % boost::lexical_cast<std::string>(*_daycount) % boost::lexical_cast<std::string>(d1) % boost::lexical_cast<std::string>(result) % t1 % t2 % ihr).c_str());
                    return result;
                } catch (std::out_of_range) {
                    return Date::MAX;
                }
            }
        }

        HazardRateMultiplier AnchoredHazardCurve::calc_hazard_rate_multiplier(const RelativeRiskValue& relative_risk_value) const {
            const Date start = relative_risk_value.ref_start;
            const Date end = relative_risk_value.ref_end;               
            const double ih = integrated_hazard_rate(start, end);
            const double base = HazardCurve::jump_probability(ih);
            if (base == 0) {
                throw std::runtime_error("HazardModel: zero base probability");
            }
            const double observed = base * relative_risk_value.relative_risk;
            if (observed >= 1) {
                throw std::runtime_error("HazardModel: observed probability >= 1");
            }
            const double numerator = HazardCurve::integrated_hazard_rate_from_jump_proba(observed);
            const double multiplier_value = numerator != ih ? numerator / ih : 1.0; // so that if ih == numerator == infinity, we don't get NaN
            if (multiplier_value != 1) {
                switch (relative_risk_value.type) {
                case RelativeRiskValue::Type::FIXED:
                    return HazardRateMultiplier(multiplier_value, start, end, false);
				case RelativeRiskValue::Type::MOVABLE:
					return HazardRateMultiplier(multiplier_value, start, end, true);
                case RelativeRiskValue::Type::SCALABLE:
                    return HazardRateMultiplier(multiplier_value);
                default:
                    throw std::logic_error(boost::str(boost::format("HazardModel: unknown RelativeRiskValue type: %d") % static_cast<int>(relative_risk_value.type)));
                }
            } else {
                return HazardRateMultiplier(); // for faster processing
            }
        }

        std::vector<HazardRateMultiplier> AnchoredHazardCurve::calc_hazard_rate_multiplier(const std::vector<RelativeRiskValue>& relative_risk_values) const {
            std::vector<HazardRateMultiplier> result;
            result.reserve(relative_risk_values.size());
            for (RelativeRiskValue relative_risk_value: relative_risk_values) {
                result.push_back(calc_hazard_rate_multiplier(relative_risk_value));
            }
            return result;
        }

        double AnchoredHazardCurve::divide_by_hazard_rate_multiplier(Date start, double expected_multiplied_ihr, const HazardRateMultiplier& hazard_rate_multiplier) const {
            if (start < _start) {
                throw std::out_of_range("AnchoredHazardCurve: start date out of range");
            }
            if (expected_multiplied_ihr < 0) {
                throw std::out_of_range("AnchoredHazardCurve: expected IHR negative");
            }
            return divide_by_hazard_rate_multiplier(start, expected_multiplied_ihr, &hazard_rate_multiplier, (&hazard_rate_multiplier) + 1);
        }

        double AnchoredHazardCurve::divide_by_hazard_rate_multiplier(Date start, double expected_multiplied_ihr, const std::vector<HazardRateMultiplier>& hazard_rate_multipliers) const {
            if (hazard_rate_multipliers.size() > 50) {
                // TODO: switch to other, non-recursive solution
                throw std::runtime_error("AnchoredHazardCurve: more than 50 multipliers unsupported");
            }
            if (start < _start) {
                throw std::out_of_range("AnchoredHazardCurve: start date out of range");
            }
            if (expected_multiplied_ihr < 0) {
                throw std::out_of_range("AnchoredHazardCurve: expected IHR negative");
            }
            return divide_by_hazard_rate_multiplier(start, expected_multiplied_ihr, hazard_rate_multipliers.begin(), hazard_rate_multipliers.end());
        }

        template <class I> double AnchoredHazardCurve::divide_by_hazard_rate_multiplier(Date start, double expected_multiplied_ihr, I begin, I end) const {
            assert(expected_multiplied_ihr >= 0);
            assert(start >= _start);
            if (expected_multiplied_ihr == 0) {
                return 0;
            }
            if (begin == end) {
                return expected_multiplied_ihr;
            } else {
                const HazardRateMultiplier& hrm = *begin;
                const I next = begin + 1;
                if (hrm.value != 1) {
                    if (start >= hrm.from && hrm.to == Date::POS_INF) {
                        const double intermediate = safe_divide(expected_multiplied_ihr, hrm.value);
                        return divide_by_hazard_rate_multiplier(start, intermediate, next, end);
                    } else if (hrm.from == hrm.to) {
                        // This multiplier is a no-op
                        return divide_by_hazard_rate_multiplier(start, expected_multiplied_ihr, next, end);
                    } else {
                        double result_ihr = 0;
                        if (hrm.from > start) {
                            const double front_ihr = integrated_hazard_rate(start, hrm.from, next, end);
                            if (front_ihr > expected_multiplied_ihr) {
                                // expected_multiplied_ihr is reached before hrm starts working
                                return divide_by_hazard_rate_multiplier(start, expected_multiplied_ihr, next, end);
                            } else {
                                if (next == end) {
                                    result_ihr = front_ihr;
                                } else {
                                    result_ihr = integrated_hazard_rate(start, hrm.from);
                                }
                                expected_multiplied_ihr = expected_multiplied_ihr - front_ihr;
                            }                            
                        }
                        start = std::max(start, hrm.from);
                        if (hrm.to.is_pos_infinity()) {
                            const double intermediate = safe_divide(expected_multiplied_ihr, hrm.value);
                            result_ihr += divide_by_hazard_rate_multiplier(start, intermediate, next, end);
                            return result_ihr;
                        } else {
                            if (start < hrm.to) {
                                const double middle_ihr = integrated_hazard_rate(start, hrm.to, next, end);
                                const double multiplied_middle_ihr = middle_ihr * hrm.value;
                                if (multiplied_middle_ihr >= expected_multiplied_ihr) {
                                    // expected_multiplied_ihr has to be reached not after hrm.from
                                    const double intermediate = safe_divide(expected_multiplied_ihr, hrm.value);
                                    result_ihr += divide_by_hazard_rate_multiplier(start, intermediate, next, end);
                                    return result_ihr;
                                } else {
                                    if (next == end) {
                                        result_ihr += middle_ihr;
                                    } else {
                                        result_ihr += integrated_hazard_rate(start, hrm.to);
                                    }
                                    expected_multiplied_ihr = expected_multiplied_ihr - multiplied_middle_ihr;                
                                }
                            }
                        }
                        start = std::max(start, hrm.to);
                        return result_ihr + divide_by_hazard_rate_multiplier(start, expected_multiplied_ihr, next, end);
                    }
                } else {
                    return divide_by_hazard_rate_multiplier(start, expected_multiplied_ihr, next, end);
                }
            }
        }

        double AnchoredHazardCurve::safe_divide(const double ihr, const double multiplier) {
            assert(ihr >= 0);
            assert(multiplier >= 0);
            if (std::isinf(ihr) && std::isinf(multiplier)) {
                return 1E-12;
            } else if (ihr == 0.0 && multiplier == 0.0) {
                return 0.0;
            } else {
                return ihr / multiplier;
            }
        }

		static std::vector<HazardRateMultiplier> move_multipliers(const std::vector<HazardRateMultiplier>& multipliers, Date old_start, Date new_start) {
			std::vector<HazardRateMultiplier> new_multipliers;
			new_multipliers.reserve(multipliers.size());
			const Period delta(old_start, new_start);
			for (const auto& hrm : multipliers) {
				if (hrm.movable) {
					new_multipliers.push_back(HazardRateMultiplier(hrm.value, hrm.from + delta, hrm.to + delta, hrm.movable));
				} else {
					new_multipliers.push_back(hrm);
				}
			}
			return new_multipliers;
		}

		static void apply_multipliers(std::unique_ptr<HazardCurve>& hc, const std::vector<HazardRateMultiplier>& multipliers, Date start, const Daycount& daycount) {
			for (const HazardRateMultiplier& hrm : multipliers) {
				const Date d0 = std::max(start, hrm.from);
				const Date d1 = std::max(start, hrm.to);
				const double t0 = d0.is_pos_infinity() ? std::numeric_limits<double>::infinity() : daycount.calc(start, d0);
				const double t1 = d1.is_pos_infinity() ? std::numeric_limits<double>::infinity() : daycount.calc(start, d1);
				hc = hc->multiply_hazard_rate(t0, t1, hrm.value);
			}
		}

        /** AnchoredHazardCurve constructed from data described with periods */
        class AnchoredHazardCurveFromPeriods: public AnchoredHazardCurve {
        public:
			AnchoredHazardCurveFromPeriods(Date start,
				std::shared_ptr<const Daycount> daycount,
				std::shared_ptr<const HazardCurveFactory> hazard_curve_factory,
				const std::vector<Period>& periods,
				const std::vector<double>& jump_probabilities,
				bool periods_additive,
				bool conditional, const std::vector<HazardRateMultiplier>& multipliers)
                : AnchoredHazardCurve(start, daycount, build_hazard_curve(start, daycount, hazard_curve_factory, periods, jump_probabilities, periods_additive, conditional, multipliers)),
                  _hazard_curve_factory(hazard_curve_factory), _periods(periods), _jump_probabilities(jump_probabilities),
                  _periods_additive(periods_additive), _conditional(conditional), _multipliers(multipliers) {
                // everything's been checked already
            }
            
            std::unique_ptr<AnchoredHazardCurve> move(Date new_start) const override {				
				return std::unique_ptr<AnchoredHazardCurve>(new AnchoredHazardCurveFromPeriods(new_start,
					_daycount,
					_hazard_curve_factory,
					_periods,
					_jump_probabilities,
					_periods_additive,
					_conditional, move_multipliers(_multipliers, start(), new_start)));
            }
        private:
			static std::unique_ptr<HazardCurve> build_hazard_curve(const Date start,
				const std::shared_ptr<const Daycount> daycount,
				const std::shared_ptr<const HazardCurveFactory> hazard_curve_factory,
				const std::vector<Period>& periods,
				const std::vector<double>& jump_probabilities,
				const bool periods_additive, const bool conditional, const std::vector<HazardRateMultiplier>& multipliers) {
                if (!daycount) {
                    throw std::domain_error("AnchoredHazardCurveFromPeriods: null daycount");
                }
                const size_t n = periods.size();
				std::vector<double> times;
				times.reserve(n);
                Date d = start;
				if (periods_additive) {
					for (const auto& period : periods) {
						d = d + period;
						times.push_back(daycount->calc(start, d));
					}
					/*for (size_t i = 0; i < n; ++i) {
						d = d + periods[i];
						times[i] = daycount->calc(start, d);
					}*/
				} else {
					for (const auto& period : periods) {
						d = start + period;
						times.push_back(daycount->calc(start, d));
					}
					/*for (size_t i = 0; i < n; ++i) {
						d = start + periods[i];
						times[i] = daycount->calc(start, d);
					}*/
				}
				auto hc = hazard_curve_factory->build(times, jump_probabilities, conditional);
				apply_multipliers(hc, multipliers, start, *daycount);
				return hc;
            }
            
			const std::shared_ptr<const HazardCurveFactory> _hazard_curve_factory;
            const std::vector<Period> _periods;
            const std::vector<double> _jump_probabilities;
			const bool _periods_additive;
			const bool _conditional;
			const std::vector<HazardRateMultiplier> _multipliers;
        };

        /** AnchoredHazardCurve constructed from data described with absolute dates */
        class AnchoredHazardCurveFromDates: public AnchoredHazardCurve {
        public:
			AnchoredHazardCurveFromDates(Date start,
				std::shared_ptr<const Daycount> daycount,
				std::shared_ptr<const HazardCurveFactory> hazard_curve_factory,
				const std::vector<Date>& end_dates,
				const std::vector<double>& jump_probabilities,
				const std::vector<HazardRateMultiplier>& multipliers)
                : AnchoredHazardCurve(start, daycount, build_hazard_curve(start, daycount, hazard_curve_factory, end_dates, jump_probabilities, multipliers)),
                  _hazard_curve_factory(hazard_curve_factory), _dates(end_dates), _jump_probabilities(jump_probabilities), _multipliers(multipliers) {
                // everything's been checked already
            }
            
            std::unique_ptr<AnchoredHazardCurve> move(Date new_start) const override {
                std::vector<Date> new_dates(_dates.size());
                const auto delta = new_start - start();
                std::transform(_dates.begin(), _dates.end(), new_dates.begin(), [&delta](const Date& d) { return d + delta; });
                return std::unique_ptr<AnchoredHazardCurve>(new AnchoredHazardCurveFromDates(new_start,
                                                                                               _daycount,
                                                                                               _hazard_curve_factory,
                                                                                               new_dates,
                                                                                               _jump_probabilities, move_multipliers(_multipliers, start(), new_start)));
            }
        private:
            static std::unique_ptr<HazardCurve> build_hazard_curve(Date start,
                                                                   std::shared_ptr<const Daycount> daycount,
                                                                   std::shared_ptr<const HazardCurveFactory> hazard_curve_factory,
                                                                   const std::vector<Date>& dates,
                                                                   const std::vector<double>& jump_probabilities,
				const std::vector<HazardRateMultiplier>& multipliers) {
                if (!daycount) {
                    throw std::domain_error("AnchoredHazardCurveFromDates: null daycount");
                }
                const size_t n = dates.size();
                std::vector<double> times(n);
                for (size_t i = 0; i < n; ++i) {
                    times[i] = daycount->calc(start, dates[i]);
                }
                auto hc = hazard_curve_factory->build(times, jump_probabilities, false);
				apply_multipliers(hc, multipliers, start, *daycount);
				return hc;
            }
            
			const std::shared_ptr<const HazardCurveFactory> _hazard_curve_factory;
            const std::vector<Date> _dates;
            const std::vector<double> _jump_probabilities;
			const std::vector<HazardRateMultiplier> _multipliers;
        };

		class AnchoredHazardCurveSimple : public AnchoredHazardCurve {
		public:
			AnchoredHazardCurveSimple(Date start, std::shared_ptr<const Daycount> daycount, std::unique_ptr<const HazardCurve>&& hazard_curve)
				: AnchoredHazardCurve(start, daycount, std::move(hazard_curve)) {}
			std::unique_ptr<AnchoredHazardCurve> move(Date new_start) const override {
				return std::make_unique<AnchoredHazardCurveSimple>(new_start, _daycount, _hazard_curve->clone());
			}
		};

        std::unique_ptr<AnchoredHazardCurve> AnchoredHazardCurve::build(Date start, std::shared_ptr<const Daycount> daycount,
                                                                        std::shared_ptr<const HazardCurveFactory> hazard_curve_factory,
                                                                        const std::vector<Period>& periods,
                                                                        const std::vector<double>& jump_probabilities,
                                                                        bool periods_additive, bool conditional,
			const std::vector<HazardRateMultiplier>& multipliers) {
            return std::unique_ptr<AnchoredHazardCurve>(new AnchoredHazardCurveFromPeriods(start, daycount, hazard_curve_factory,
                                                                                           periods, jump_probabilities, periods_additive, conditional, multipliers));
        }

        std::unique_ptr<AnchoredHazardCurve> AnchoredHazardCurve::build(Date start,
                                                                        std::shared_ptr<const Daycount> daycount,
                                                                        std::shared_ptr<const HazardCurveFactory> hazard_curve_factory,
                                                                        const std::vector<Date>& end_dates,
                                                                        const std::vector<double>& jump_probabilities,
			const std::vector<HazardRateMultiplier>& multipliers) {
            return std::unique_ptr<AnchoredHazardCurve>(new AnchoredHazardCurveFromDates(start, daycount, hazard_curve_factory,
                                                                                         end_dates, jump_probabilities, multipliers));
        }

		std::unique_ptr<AnchoredHazardCurve> AnchoredHazardCurve::build(Date start, std::shared_ptr<const Daycount> daycount, std::unique_ptr<HazardCurve>&& hazard_curve) {
			return std::make_unique<AnchoredHazardCurveSimple>(start, daycount, std::move(hazard_curve));
		}
    }
}
