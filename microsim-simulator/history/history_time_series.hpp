/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_HISTORY_TIME_SERIES_H
#define __AVERISERA_MS_HISTORY_TIME_SERIES_H

#include "../history.hpp"
#include "../history_data.hpp"
#include "core/log.hpp"
#include "core/math_utils.hpp"
#include "core/printable.hpp"
#include "core/time_series.hpp"
#include <limits>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
    namespace microsim {
        /* \brief History backed by a TimeSeries
         * 
         * \tparam V Value type: double, float or integer type
         */
        template <class V> class HistoryTimeSeries : public History {
//            static_assert(std::is_same<double, typename std::enable_if<std::is_floating_point<V>::value, V>::type
        public:
            HistoryTimeSeries(const std::string& name)
			: name_(name) {                
            }			

            HistoryTimeSeries(const std::string& name, const TimeSeries<Date, V>& data) 
				: name_(name) {
                MathUtils::safe_cast<index_t>(data.size());
                _ts.reserve(data.size());
                for (auto it = data.begin(); it != data.end(); ++it) {
                    append_impl(it->first, it->second);
                }                
            }
            
            bool empty() const {
                return _ts.empty();
            }
            
            Date last_date() const {
                return _ts.last_time();
            }
            
            Date last_date(Date asof) const {
                if (empty()) {
                    throw std::logic_error("HistoryTimeSeries: history is empty");
                }
                const auto pd = _ts.last_time(asof);
                if (pd) {
                    return *pd;
                } else {
                    throw std::out_of_range(boost::str(boost::format("HistoryTimeSeries: no dates on or before %s") % boost::lexical_cast<std::string>(asof)));
                }
            }
            
            Date first_date() const {
                return _ts.first_time();
            }
            
            double_t as_double(Date as_of) const {
                const V* value = _ts.value(as_of);
                if (value != nullptr) {
                    return static_cast<double_t>(*value);
                } else {
                    return std::numeric_limits<double_t>::quiet_NaN();
                }
            }
            
            int_t as_int(Date as_of) const {
                const V* value = _ts.value(as_of);
                if (value != nullptr) {
                    return static_cast<int_t>(*value);
                } else {
                    throw std::runtime_error(boost::str(boost::format("HistoryTimeSeries: no value on %s") % boost::lexical_cast<std::string>(as_of)));
                }
            }
            
            double_t last_as_double() const {
                return static_cast<double_t>(_ts.last_value());
            }
            
            int_t last_as_int() const {
                return static_cast<int_t>(_ts.last_value());
            }
            
            double_t last_as_double(Date as_of) const {
                const V* value = _ts.last_value(as_of);
                if (value != nullptr) {
                    return static_cast<double_t>(*value);
                } else {
                    throw std::out_of_range(boost::str(boost::format("HistoryTimeSeries: no dates on or before %s") % boost::lexical_cast<std::string>(as_of)));
                }
            }
            
            int_t last_as_int(Date as_of) const {
                const V* value = _ts.last_value(as_of);
                if (value != nullptr) {
                    return static_cast<int_t>(*value);
                } else {
                    throw std::out_of_range(boost::str(boost::format("HistoryTimeSeries: no dates on or before %s") % boost::lexical_cast<std::string>(as_of)));
                }
            }
            
            void append(Date date, double_t value) override {
                append_impl(date, value);
            }
            
            void append(Date date, int_t value) override {
                append_impl(date, value);
            }

            void correct(double_t value) override {
                if (_ts.empty()) {
                    throw std::domain_error("HistoryTimeSeries: History is empty");
                } else {
                    const index_t idx = _ts.size() - 1;
                    _ts.value_at_index(idx) = MathUtils::safe_cast<V>(value);
                }
            }

            index_t size() const override {
                return _ts.size();
            }

            Date date(index_t idx) const override {
                if (idx < size()) {
                    return _ts[idx].first;
                } else {
                    throw std::out_of_range(boost::str(boost::format("HistoryTimeSeries: index %d too large (size %d)") % idx % size()));
                }
            }

            double_t as_double(index_t idx) const override {
                if (idx < size()) {
                    return static_cast<double_t>(_ts[idx].second);
                } else {
                    throw std::out_of_range(boost::str(boost::format("HistoryTimeSeries: index %d too large (size %d)") % idx % size()));
                }
            }

            int_t as_int(index_t idx) const override {
                if (idx < size()) {
                    return static_cast<int_t>(_ts[idx].second);
                } else {
                    throw std::out_of_range(boost::str(boost::format("HistoryTimeSeries: index %d too large (size %d)") % idx % size()));
                }
            }

            index_t last_index(Date asof) const override {
                return _ts.last_index(asof);
            }

            index_t first_index(Date asof) const override {
                return _ts.first_index(asof);
            }
            
            void print(std::ostream& os) const override {
				os << name_ << " | " << print_type_name<V>() << ": " << _ts;
            }

            std::unique_ptr<History> clone() const override {
                return std::unique_ptr<History>(new HistoryTimeSeries<V>(name(), TimeSeries<Date, V>(_ts)));
            }

			HistoryData to_data() const override {
				HistoryData hd(print_type_name<V>(), name());
				hd.reserve(size());
				for (const auto& tv : _ts) {
					hd.append(tv.first, tv.second);
				}
				return hd;
			}

			const std::string& name() const override {
				return name_;
			}
        private:
            template <class V2> void append_impl(Date date, V2 value) {
                if (empty() || date > last_date()) {
                    _ts.push_back(date, MathUtils::safe_cast<V>(value));
                } else {
					const std::string msg(boost::str(boost::format("HistoryTimeSeries::append_impl: appending value %s: appended date %s not past the current last %s in history %s") % boost::lexical_cast<std::string>(value) % boost::lexical_cast<std::string>(date) % boost::lexical_cast<std::string>(last_date()) % name()));
					LOG_ERROR() << msg;
                    throw std::domain_error(msg);
                }
            }

			TimeSeries<Date, V> _ts;
			std::string name_;
        };
    }
}

#endif // __AVERISERA_MS_HISTORY_TIME_SERIES_H
