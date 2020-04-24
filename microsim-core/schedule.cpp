/*
 * (C) Averisera Ltd 2015
 */

#include "schedule.hpp"
#include "schedule_definition.hpp"
#include "core/dates.hpp"
#include <algorithm>
#include <stdexcept>
#include <boost/iterator/transform_iterator.hpp>

namespace averisera {
    namespace microsim {
        Schedule::Schedule()
            : _size(0), _nbr_dates(0) {            
        }
        
        Schedule::Schedule(const ScheduleDefinition& definition) {
            if (!valid(definition)) {
                throw std::domain_error("Schedule: definition invalid");
            }
            if (definition.start == definition.end) {
                _periods.resize(1);
                _periods[0] = period_t(definition.start, definition.end, *definition.daycount);
            } else {
                Date date = definition.start;
                auto iter_ptr = definition.start.make_date_iterator(definition.frequency);
                auto& iter = *iter_ptr;
                while (date < definition.end) {
                    ++iter;
                    Date next = *iter;
                    if (next > definition.end) {
                        next = definition.end;
                        const long remaining_days = (next - date).days();
                        const long freq_days = definition.frequency.days();
                        if (2 * remaining_days <= freq_days && !_periods.empty()) {
                            // change last period instead of adding a new one
                            _periods.back() = period_t(_periods.back().begin, next, *definition.daycount);
                            break;
                        }
                    }
                    _periods.push_back(period_t(date, next, *definition.daycount));
                    date = next;
                }
            }
            _size = _periods.size();
            if (_size == 0) {
                _nbr_dates = 0;
            } else if (_size == 1 && _periods.front().begin == _periods.front().end) {
                _nbr_dates = 1;
            } else {
                _nbr_dates = _size + 1;
            }
        }

        Schedule::Schedule(Schedule&& other) noexcept
        : _periods(std::move(other._periods)), _size(other._size), _nbr_dates(other._nbr_dates) {
            other._size = 0;
            other._periods.resize(0);
            other._nbr_dates = 0;
        }

        Schedule::Schedule(const std::vector<Date>& dates) {
            if (dates.empty()) {
                throw std::domain_error("Schedule: no dates");
            }
            const index_t n = dates.size();
            const Daycount& daycount = *Daycount::YEAR_FRACT();
            if (n == 1) {
                _size = 1;
                _periods.push_back(SchedulePeriod(dates[0], dates[0], daycount));
            } else {
                _size = n - 1;
                for (size_t i = 1; i < n; ++i) {
                    if (dates[i] > dates[i - 1]) {
                        _periods.push_back(SchedulePeriod(dates[i - 1], dates[i], daycount));
                    } else {
                        throw std::domain_error("Schedule: dates not strictly increasing");
                    }
                }
            }
            _nbr_dates = dates.size();
        }
        
        bool Schedule::valid(const ScheduleDefinition& definition) {
            if (definition.start.is_not_a_date()) {
                return false;
            }
            if (definition.end.is_not_a_date()) {
                return false;
            }
            if (definition.end < definition.start) {
                return false;
            }
            if (definition.end > definition.start) {
                // need a positive frequency
                if (definition.frequency.days() <= 0) {
                    return false;
                }
            }
            if (!definition.daycount) {
                return false;
            }
            return true;
        }
        
        Date Schedule::date(index_t idx) const {
			assert(idx < nbr_dates());
            /*if (idx >= nbr_dates()) {
                throw std::out_of_range("Schedule: index too large");
            }*/
            if (idx < size()) {
                return _periods[idx].begin;
            } else {
                return _periods.back().end;
            }
        }

        struct period_to_start {
            Date operator()(const SchedulePeriod& period) const {
                return period.begin;
            }
        };

        bool Schedule::contains(Date date) const {
            if (empty()) {
                return false;
            } else {
                if (date == _periods.back().end) {
                    return true;
                } else {
                    return std::binary_search(boost::make_transform_iterator(_periods.begin(), period_to_start()), boost::make_transform_iterator(_periods.end(), period_to_start()), date);
                }
            }
        }

        bool Schedule::contains(const Schedule& other) const {
            const index_t n = other.nbr_dates();
            for (index_t i = 0; i < n; ++i) {
                if (!contains(other.date(i))) {
                    return false;
                }
            }
            return true;
        }

		Schedule::index_t Schedule::index(const Date date) const {
            if (!empty()) {
                const auto it = std::lower_bound(_periods.begin(), _periods.end(), date, [](const SchedulePeriod& p, Date d) { return p.begin < d; });
                if (it != _periods.end() && it->begin == date) {
                    return static_cast<index_t>(std::distance(_periods.begin(), it));
                } else if (date == _periods.back().end) {
                    return size();
                }
            }
            throw std::out_of_range("Schedule::index: date not in schedule");
        }

		Schedule::index_t Schedule::find_containing_period(const Date date) const {
			if (!empty()) {
				const auto it = std::lower_bound(_periods.begin(), _periods.end(), date, [](const SchedulePeriod& p, Date d) { return p.end <= d; });
				if (it != _periods.end()) {
					return static_cast<index_t>(std::distance(_periods.begin(), it));
				} else {
					throw std::out_of_range("Schedule::find_containing_period: date not in schedule");
				}
			} else {
				throw std::out_of_range("Schedule::find_containing_period: empty schedule");
			}
		}

		bool Schedule::operator==(const Schedule& other) const {
			if (this != &other) {
				return _periods == other._periods;
			} else {
				return true;
			}
		}
    }
}
