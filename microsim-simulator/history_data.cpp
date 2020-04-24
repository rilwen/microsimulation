#include "history_data.hpp"
#include "history_factory.hpp"
#include "core/dates.hpp"
#include "core/period.hpp"
#include "core/time_series.hpp"
#include "core/stl_utils.hpp"
#include <algorithm>

namespace averisera {
	namespace microsim {
		HistoryData::HistoryData(const std::string& factory_type, const std::string& name)
			: factory_type_(factory_type),
			name_(name) {
			const auto value_type_name = HistoryFactory::from_string(factory_type).second;
			values_ = ObjectVector(type_from_string(value_type_name));
		}

		HistoryData::HistoryData(HistoryData&& other) noexcept
		: values_(std::move(other.values_)),
			dates_(std::move(other.dates_)),
			factory_type_(std::move(other.factory_type_)),
			name_(std::move(other.name_))
		{}

		HistoryData& HistoryData::operator=(HistoryData&& other) noexcept {
			if (this != &other) {
				HistoryData clone(std::move(other));
				this->swap(clone);
			}
			return *this;
		}

		HistoryData::HistoryData(const HistoryData& other)
			: values_(other.values_),
			dates_(other.dates_),
			factory_type_(other.factory_type_),
			name_(other.name_)
		{
		}

		HistoryData& HistoryData::operator=(const HistoryData& other) {
			if (this != &other) {
				HistoryData clone(other);
				this->swap(clone);
			}
			return *this;
		}

		void HistoryData::swap(HistoryData& other) noexcept {
			values_.swap(other.values_);
			dates_.swap(other.dates_);
			factory_type_.swap(other.factory_type_);
			name_.swap(other.name_);
		}

		HistoryData::date_iterator HistoryData::find_by_date(Date date) {
			date_iterator it = std::lower_bound(dates_.begin(), dates_.end(), date);
			if (it != dates_.end() && *it == date) {
				return it;
			} else {
				return dates_.end();
			}
		}

		HistoryData::const_date_iterator HistoryData::find_by_date(Date date) const {
			const_date_iterator it = std::lower_bound(dates_.begin(), dates_.end(), date);
			if (it != dates_.end() && *it == date) {
				return it;
			} else {
				return dates_.end();
			}
		}

		bool HistoryData::change_date_safely(const Date old_date, const Date new_date) {
			if (new_date == old_date) {
				return find_by_date(old_date) != dates_.end();
			}
			const date_iterator old_it = find_by_date(old_date);
			if (old_it == dates_.end()) {
				return false;
			}
			if (new_date < old_date) {
				if (old_it > dates_.begin()) {
					const const_date_iterator prev_it = old_it - 1;
					if (*prev_it < new_date) {
						*old_it = new_date;
						return true;
					} else {
						return false;
					}
				} else {
					*old_it = new_date;
					return true;
				}
			} else {
				const const_date_iterator next_it = old_it + 1;
				if (next_it < dates_.end()) {
					if (*next_it > new_date) {
						*old_it = new_date;
						return true;
					} else {
						return false;
					}
				} else {
					*old_it = new_date;
					return true;
				}
			}
		}

		void HistoryData::clear() {
			values_ = ObjectVector();
			dates_ = std::vector<Date>();
			factory_type_ = "";
			name_ = "";
		}

		template <class V> size_t HistoryData::append_impl(const std::string& str) {
			const TimeSeries<Date, V> ts(TimeSeries<Date, V>::from_string(str));
			for (const auto& date_value : ts) {
                if (dates_.empty() || date_value.first > dates_.back()) {
                    dates_.push_back(date_value.first);
                    values_.push_back(date_value.second);
                } else {
                    throw std::runtime_error("HistoryData: input dates out of order");
                }
			}
			return ts.size();
		}

        size_t HistoryData::append(const std::string& str) {
            if (!str.empty()) {
                switch (str[0]) {
                case 'D':
                    return append_impl<ObjectVector::double_t>(str.substr(1));
                case 'I':
                    return append_impl<ObjectVector::int_t>(str.substr(1));
                default:
                    throw std::runtime_error("HistoryData: cannot parse string with history data");
                }
            } else {
                return 0;
            }
        }

		void HistoryData::shift_dates(const Period& delta) {
			size_t max_cnt = 0;
			size_t min_cnt = 0;
			for (Date& dt : dates_) {
				try {
					dt = dt + delta;
				} catch (std::out_of_range&) {
					// overflow detected... unlikely!
					if (delta.size > 0) {
						dt = Date::MAX;
						++max_cnt;
					} else {
						dt = Date::MIN;
						++min_cnt;
					}
				}
			}
			if (max_cnt > 1 || min_cnt > 1) {
				throw std::runtime_error("HistoryData: shifting dates is not possible");
			}
		}

		std::string HistoryData::value_typ_to_str(type_t typ) {
			std::stringstream ss;
			ss << typ;
			return ss.str();
		}

        std::ostream& operator<<(std::ostream& os, const HistoryData& data) {
			os << "{NAME=" << data.name_ << "|";
            os << "DATES=" << data.dates_ << "|";
            os << "VALUES=" << data.values_ << "|";
            os << "FACTORY_TYPE=" << data.factory_type_ << "}";
            return os;
        }
	}
}
