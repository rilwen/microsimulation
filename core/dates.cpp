/*
 *  (C) Averisera Ltd 2015
 */
#include "dates.hpp"
#include "log.hpp"
#include "math_utils.hpp"
#include "period.hpp"
#include "preconditions.hpp"
#include <climits>
#include <stdexcept>
#include <boost/date_time/gregorian_calendar.hpp>
#include <boost/format.hpp>

namespace averisera {
    const Date Date::NAD = Date();
    const Date Date::POS_INF = Date(boost::gregorian::pos_infin);
    const Date Date::NEG_INF = Date(boost::gregorian::neg_infin);
	const Date::year_type Date::MIN_YEAR = 1400;
    const Date Date::MIN = Date(Date::MIN_YEAR, 1, 1);
	const Date::year_type Date::MAX_YEAR = 9999;
    const Date Date::MAX = Date(Date::MAX_YEAR, 12, 31);

#ifndef NDEBUG
	Date::Date(year_type year, month_type month, day_type day) {
		if (year < MIN_YEAR || year > MAX_YEAR) {
			throw std::out_of_range(boost::str(boost::format("Date: year %d outside allowed range [%d, %d]") % year % MIN_YEAR % MAX_YEAR));
		}
		if (month < 1 || month > 12) {
			throw std::out_of_range(boost::str(boost::format("Date: month %d outside [1, 12]") % month));
		}
		impl_ = boost::gregorian::date(year, month, day);
	}
#endif // NDEBUG

	Period Date::operator-(Date other) const {
		return Period(PeriodType::DAYS, other.dist_days(*this));
	}
        
    std::unique_ptr<boost::gregorian::date_iterator> Date::make_date_iterator(const Period& period) const {
        switch (period.type) {
        case PeriodType::NONE:
            return nullptr;
        case PeriodType::DAYS:
            return std::unique_ptr<boost::gregorian::date_iterator>(new boost::gregorian::day_iterator(impl_, period.size));
        case PeriodType::WEEKS:
            return std::unique_ptr<boost::gregorian::date_iterator>(new boost::gregorian::week_iterator(impl_, period.size));
        case PeriodType::MONTHS:
            return std::unique_ptr<boost::gregorian::date_iterator>(new boost::gregorian::month_iterator(impl_, period.size));
        case PeriodType::YEARS:
            return std::unique_ptr<boost::gregorian::date_iterator>(new boost::gregorian::year_iterator(impl_, period.size));
        default:
            throw std::logic_error("Date::make_date_iterator: Unknown Period type");
        }
    }

    bool Date::is_leap(year_type year) {
        return boost::gregorian::gregorian_calendar::is_leap_year(year);
    }

	static Period::size_type dist_any_period(Date d1, const Date d2, const Period& period) {
		assert(d1 < d2);
		Period::size_type cnt = 0;
		while (d1 < d2) {
			++cnt; // TODO: handle overflow
			d1 = d1 + period;
		}
		return d1 > d2 ? (cnt - 1) : cnt;
	}

	static Period::size_type dist_years_(Date d1, Date d2) {
		assert(d1 < d2);
		const auto diff_days = d1.dist_days(d2);
		//LOG_TRACE() << "diff_days=" << diff_days;
		assert(diff_days >= 0);
		if (diff_days > 0) {
			const auto diff_years_dbl = static_cast<double>(diff_days) / 365.25;
			//LOG_TRACE() << "diff_years_dbl=" << diff_years_dbl;
			assert(diff_years_dbl >= 0.0);
			const auto diff_years_rnd= std::round(diff_years_dbl);
			assert(diff_years_rnd >= 0);
			if (std::abs(diff_years_dbl - diff_years_rnd) > 0.2) {
				return static_cast<Period::size_type>(diff_years_dbl); // floor it
			} else {
				// try the slow version
				const auto diff_years = static_cast<Period::size_type>(d2.year() - d1.year());
				assert(diff_years >= 0);
				if (d1 + Period::years(diff_years) <= d2) {
					return diff_years;
				} else {
					assert(diff_years > 0);
					return diff_years - 1;
				}
			}
		} else {
			return 0;
		}
	}

	Period::size_type Date::dist_years(Date other) const {
		if (other > *this) {
			return dist_years_(*this, other);
		} else if (other == *this) {
			return 0;
		} else {
			return -dist_years_(other, *this);
		}
	}

	Period::size_type Date::dist_days(Date other) const {
		return static_cast<Period::size_type>((other.impl_ - impl_).days());
	}

	Period::size_type Date::dist(Date other, const Period& period) const {
		check_that(period.size > 0, "Date::dist: period size must be positive");
		if (other > *this) {
			switch (period.type) {
			case PeriodType::NONE:
				throw std::domain_error("Date::dist: period type cannot be NONE");
			case PeriodType::DAYS:
				return (other - *this).size / period.size;
			case PeriodType::WEEKS:
				// week has always 7 days
				return (other - *this).size / (7 * period.size);
			case PeriodType::MONTHS:
				return dist_any_period(*this, other, period);
			case PeriodType::YEARS:
				return dist_years_(*this, other) / period.size;
			default:
				throw std::logic_error("Date:dist: Unknown Period type");
			}
		} else if (other == *this) {
			return 0;
		} else {
			return - other.dist(*this, period);
		}
	}

	Date operator+(Date date, const Period& period) {
        if (date.is_not_a_date()) {
            return date;
        }
		try {
			switch (period.type) {
			case PeriodType::NONE:
				return date;
			case PeriodType::DAYS:
			{
				const auto year = date.year();
				if (period.size > 366 * (1 + Date::MAX_YEAR - year) || period.size < 366 * (Date::MIN_YEAR - year - 1)) {
					throw std::out_of_range(boost::str(boost::format("Date::operator+: adding %d days moves date %s out of range") % period.size % date));
				}
			}
				return Date(date.impl_ + boost::gregorian::days(period.size));
			case PeriodType::WEEKS:
			{
				const auto year = date.year();
				if (period.size > 53 * (1 + Date::MAX_YEAR - year) || period.size < 53 * (Date::MIN_YEAR - year - 1)) {
					throw std::out_of_range(boost::str(boost::format("Date::operator+: adding %d weeks moves date %s out of range") % period.size % date));
				}
			}
				return Date(date.impl_ + boost::gregorian::weeks(period.size));
			case PeriodType::MONTHS:
			{
				const auto year = date.year();
				if (period.size > 12 * (1 + Date::MAX_YEAR - year) || period.size < 12 * (Date::MIN_YEAR - year - 1)) {
					throw std::out_of_range(boost::str(boost::format("Date::operator+: adding %d months moves date %s out of range") % period.size % date));
				}
			}
				return Date(date.impl_ + boost::gregorian::months(period.size));
			case PeriodType::YEARS:
			{
				const auto year = date.year();
				if (period.size > (Date::MAX_YEAR - year) || period.size < (Date::MIN_YEAR - year)) {
					throw std::out_of_range(boost::str(boost::format("Date::operator+: adding %d years moves date %s out of range") % period.size % date));
				}
			}
				return Date(date.impl_ + boost::gregorian::years(period.size));
			default:
				throw std::logic_error("Period: Unknown type");
			}
		} catch (std::out_of_range& e) {
			throw e;
		} catch (std::logic_error& e) {
			throw e;
		} catch (std::exception& e) {
			throw std::out_of_range(boost::str(boost::format("Date::operator+: adding period %s moves date %s out of range: %s") % period % date % e.what()));
		}
    }

    Date operator-(Date date, const Period& period) {
        Period p(period);
        p.size *= -1; // we have a copy
        return date + p;
    }
}
