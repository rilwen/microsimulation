/*
* (C) Averisera Ltd 2015
*/
#include "hazard_curve_piecewise_constant.hpp"
#include "core/log.hpp"
#include <algorithm>
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
        HazardCurvePiecewiseConstant::HazardCurvePiecewiseConstant(const TimeSeries<double, double>& rates)
            : _rates(rates) {
			//TRACE() << "HazardCurvePiecewiseConstant::HazardCurvePiecewiseConstant: rates=" << _rates;
            construct();                
        }
        
        HazardCurvePiecewiseConstant::HazardCurvePiecewiseConstant(TimeSeries<double, double>&& rates)
        : _rates(std::move(rates)) {
			//TRACE() << "HazardCurvePiecewiseConstant::HazardCurvePiecewiseConstant: rates=" << _rates;
            construct();
        }
			
		double HazardCurvePiecewiseConstant::instantaneous_hazard_rate(double t) const {
			assert(t >= 0);
			return *_rates.last_value(t);
		}

		double HazardCurvePiecewiseConstant::average_hazard_rate(double t1, double t2) const {
			assert(t1 >= 0);
			assert(t2 >= t1);
			if (t1 == t2) {
				return instantaneous_hazard_rate(t1);
			} else {
				return integrated_hazard_rate(t1, t2) / (t2 - t1);
			}
		}

		double HazardCurvePiecewiseConstant::integrated_hazard_rate(double t1, double t2) const {
			//TRACE() << "HazardCurvePiecewiseConstant::integrated_hazard_rate: t1=" << t1 << ", t2=" << t2 << ", dt=" << (t2 - t1);
			assert(t1 >= 0);
			assert(t2 >= t1);
			const index_t i1 = _rates.last_index(t1);
			const index_t i2 = _rates.last_index(t2);
			assert(i1 <= i2);
			if (i1 < i2) {
                const index_t i2m1 = i2 - 1;
                const double t1ceil = _rates[i1 + 1].first;
                const auto& r2 = _rates[i2];
                const double t2floor = i2m1 != i1 ? r2.first : t1ceil;
                double sum = _rates[i1].second * (t1ceil - t1) + r2.second * (t2 - t2floor);
                if (i2m1 > i1) {
                    sum += _ihr[i2].second - _ihr[i1 + 1].second;
                }
				return sum;
			} else {
                return _rates[i1].second * (t2 - t1);
			}
		}

		std::unique_ptr<HazardCurve> HazardCurvePiecewiseConstant::slide(double delta) const {
            if (delta < 0) {
                throw std::out_of_range("HazardCurvePiecewiseConstant: negative delta");
            }
            const index_t i = _rates.last_index(delta);
            assert(i < size());
			std::vector<std::pair<double, double> > new_rates(size() - i);
            std::copy(_rates.begin() + i, _rates.end(), new_rates.begin());
			if (_rates[i].first == delta) {
                for (auto it = new_rates.begin(); it != new_rates.end(); ++it) {
                    it->first -= delta;
                }
            } else {
                for (auto it = new_rates.begin() + i; it != new_rates.end(); ++it) {
                    it->first -= delta;
                }
            }			
            new_rates[0].first = 0.;
			return std::unique_ptr<HazardCurve>(new HazardCurvePiecewiseConstant(std::move(TimeSeries<double, double>(std::move(new_rates)))));
		}

        double HazardCurvePiecewiseConstant::calc_t2(double t1, double ihr) const {
            assert(t1 >= 0);
            assert(ihr >= 0);
            if (ihr == 0) {
                return t1;
            }
            const double prev_ihr = integrated_hazard_rate(0, t1);
            const double total_ihr = prev_ihr + ihr;
            assert(total_ihr >= 0);
            if (total_ihr > _ihr.last_value()) {
                const double delta_ihr = total_ihr - _ihr.last_value();
                return _rates.last_time() + delta_ihr / _rates.last_value();
            } else {
                // find value, not time
                const auto it = std::lower_bound(_ihr.begin(), _ihr.end(), total_ihr, [](const std::pair<double, double>& p, double x){ return p.second < x; });
                assert(it != _ihr.end());
                const double ihr1 = it->second;
                const index_t ihr_idx = static_cast<index_t>(std::distance(_ihr.begin(), it));
                double t2;
                if (ihr1 == total_ihr) {
                    // exact match
                    t2 = _ihr[ihr_idx].first;
                } else {                    
                    const double time1 = _ihr[ihr_idx].first;
                    double ihr0;
                    double time0;
                    if (ihr_idx > 0) {
                        time0 = _ihr[ihr_idx - 1].first;
                        ihr0 = (*(it - 1)).second;
                    } else {
                        time0 = 0;
                        ihr0 = 0;
                    }       
                    assert(total_ihr >= ihr0);
                    assert(total_ihr <= ihr1);
                    assert(ihr1 > ihr0); // otherwise we would have total_ihr == ihr1
                    assert(time1 > time0);
                    t2 = (time1 * (total_ihr - ihr0) + time0 * (ihr1 - total_ihr)) / (ihr1 - ihr0);                    
                }
                return std::max(t1, t2);
            }
        }

		void HazardCurvePiecewiseConstant::validate() const {
			if (_rates.empty()) {
				throw std::domain_error("HazardCurvePiecewiseConstant: rates time series empty");
			}
			if (_rates.first_time() != 0) {
				throw std::domain_error("HazardCurvePiecewiseConstant: first time must be zero");
			}
            for (const auto& tv : _rates) {
                if (tv.second < 0) {
                    throw std::domain_error(boost::str(boost::format("HazardCurvePiecewiseConstant: negative hazard rate: %g at %g") % tv.second % tv.first));
                }
            }
		}
		
        void HazardCurvePiecewiseConstant::recalc_ihr(const index_t from) {
            index_t ihr_from = from + 1;
            if (ihr_from < _ihr.size()) {
                auto prev = _ihr[from];
                while (ihr_from < _ihr.size()) {
                    const double t = _ihr[ihr_from].first;
                    const double dt = t - prev.first;
                    assert(dt >= 0);
                    const double h = _rates[ihr_from - 1].second;
                    assert(h >= 0);
                    prev.second += h * dt;
                    _ihr.value_at_index(ihr_from) = prev.second;
                    ++ihr_from;
                    prev.first = t;
                }
            }
        }
        
        void HazardCurvePiecewiseConstant::construct() {
            validate();
            _ihr = _rates;
            _ihr.value_at_index(0) = 0;
            assert(_ihr[0].first == 0.);
            recalc_ihr(0);
        }

		std::unique_ptr<HazardCurve> HazardCurvePiecewiseConstant::multiply_hazard_rate(const double t0, const double t1, const double factor) const {
			validate(t0, t1, factor);
			if (t0 != t1 && factor != 1) {
				if (t0 > 0 || std::isfinite(t1)) {
					typedef TimeSeries<double, double>::index_t index_t;
					const index_t i0 = _rates.last_index(t0);
					const index_t i1 = _rates.last_index(t1);
					const bool b0 = _rates[i0].first != t0;
					const bool b1 = _rates[i1].first != t1;
					const index_t new_size = _rates.size() + (b0 ? 1 : 0) + (b1 ? 1 : 0);
					std::vector<typename TimeSeries<double, double>::tvpair_t> new_rates;
					new_rates.reserve(new_size);
					new_rates.insert(new_rates.end(), _rates.begin(), _rates.begin() + (i0 + 1));
					if (b0) {
						new_rates.push_back(std::make_pair(t0, new_rates.back().second * factor));
					} else {
						new_rates.back().second *= factor;
					}
					assert(i0 < size());
					assert(i1 < size());
					assert(i0 <= i1);
					const auto mult_end = _rates.begin() + (i1 + 1);
					for (auto it = _rates.begin() + (i0 + 1); it != mult_end; ++it) {
						new_rates.push_back(std::make_pair(it->first, it->second * factor));
					}
					if (b1) {
						new_rates.push_back(std::make_pair(t1, _rates[i1].second));
					} else {
						new_rates.back().second = _rates[i1].second;
					}
					for (auto it = mult_end; it != _rates.end(); ++it) {
						new_rates.push_back(*it);
					}
					return std::make_unique<HazardCurvePiecewiseConstant>(TimeSeries<double, double>(std::move(new_rates)));
				} else {					
					return std::make_unique<HazardCurvePiecewiseConstant>(_rates * factor);
				}
			} else {
				return clone();
			}
		}
	}
}
