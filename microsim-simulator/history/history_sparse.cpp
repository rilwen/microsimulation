// (C) Averisera Ltd 2014-2020
#include "history_sparse.hpp"
#include "core/dates.hpp"
#include "core/log.hpp"
#include "core/printable.hpp"
#include <limits>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        HistorySparse::HistorySparse(std::unique_ptr<History>&& impl)
            : _impl(std::move(impl)) {
            if (!_impl) {
                throw std::domain_error("HistorySparse: Null pointer to implementation");
            }
        }
        
        Date HistorySparse::last_date() const {
            if (_last_date != Date::NAD) {
                return _last_date;
            } else {
				LOG_ERROR() << "HistorySparse:::last_date: empty history " << name();
                throw std::logic_error("HistorySparse: empty history");
            }
        }
        
        Date HistorySparse::last_date(Date asof) const {
            if (asof <= _last_date) {
                return _impl->last_date(asof);
            } else {
                return last_date();
            }
        }
        
        double HistorySparse::as_double(Date date) const {
            if (date <= _last_date) {
                try {
                    return _impl->last_as_double(date);
                } catch (std::out_of_range&) {
                    // fit interface requirements
                    return std::numeric_limits<double>::quiet_NaN();
                }
            } else {
                return std::numeric_limits<double>::quiet_NaN();
            }
        }
        
        HistorySparse::int_t HistorySparse::as_int(Date date) const {
            if (date <= _last_date) {
                try {
                    return _impl->last_as_int(date);
                } catch (std::out_of_range&) {
                    // change exception type to fit interface requirements
                    throw std::runtime_error("HistorySparse: no data for this date");
                }
            } else {
                throw std::runtime_error("HistorySparse: no data for this date");
            }
        }
        
        template <class T> void HistorySparse::do_append(Date date, T value) {
            _impl->append(date, value);
        }
        
        void HistorySparse::append(Date date, const double value) {
            _last_date = date;
            bool append = false;
            if (_impl->empty()) {
                append = true;
            } else {
                if (date > _impl->last_date()) {
                    append = value != _impl->last_as_double();					
                } else {
					LOG_ERROR() << "HistorySparse::append: date " << date << " on or before last date" << _impl->last_date() << " while appending double " << value << " to history " << name();
                    throw std::domain_error("HistorySparse: Date on or before last date");
                }
            }
            if (append) {                
                do_append(date, value);
            }
        }
        
        void HistorySparse::append(Date date, const int_t value) {
            _last_date = date;
            bool append = false;
            if (_impl->empty()) {
                append = true;
            } else {
                if (date > _impl->last_date()) {
                    append = value != _impl->last_as_int();
                } else {
					LOG_ERROR() << "HistorySparse::append: date " << date << " on or before last date" << _impl->last_date() << " while appending int " << value << " to history " << name();
                    throw std::domain_error("HistorySparse: Date on or before last date");					
                }
            }
            if (append) {
                do_append(date, value);
            }
        }

        void HistorySparse::correct(double value) {
            _impl->correct(value);
        }

        std::unique_ptr<History> HistorySparse::clone() const {
            std::unique_ptr<HistorySparse> copy(new HistorySparse(std::move(_impl->clone())));
            copy->_last_date = _last_date;
            return std::move(copy);
        }

		HistoryData HistorySparse::to_data() const {
			HistoryData data = _impl->to_data();
			data.set_factory_type(std::string("sparse ") + data.factory_type());
			return data;
		}
    }
}
