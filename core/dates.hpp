/*
  (C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_DATES_H
#define __AVERISERA_DATES_H

#include "dates_fwd.hpp"
#include <cmath>
#include <iosfwd>
#include <memory>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/functional/hash.hpp>

/** \namespace averisera
 * @brief Averisera modelling code
 */
namespace averisera {

	/** Date class - wrapper around boost::gregorian::date */
	class Date {
	public:
		typedef unsigned short int year_type; /**< Type for year number */
		typedef unsigned short int month_type; /**< Type for month number */
		typedef unsigned short int day_type; /**< Type for day number */		

		/** Not-A-Date
		Special value indicating unknown or empty date.
		*/
		static const Date NAD;

		/** Date in +Infinity */
		static const Date POS_INF;

		/** Date in -Infinity */
		static const Date NEG_INF;

		/** Minimum year supported */
		static const year_type MIN_YEAR;

		/** Minimum finite date supported */
		static const Date MIN;

		/** Maximum year supported */
		static const year_type MAX_YEAR;

		/** Minimum finite date supported */
		static const Date MAX;

		/** Creates a NaD value */
		Date() {}

		Date(boost::gregorian::date impl)
			: impl_(impl) {}

		/** Create a normal date 
		@throw std::exception If values outside supported ranges */
		Date(year_type year, month_type month, day_type day)
#ifdef NDEBUG
			: impl_(year, month, day) {
		}
#else
			;
#endif // NDEBUG

		/** Create a special value */
		explicit Date(boost::date_time::special_values v)
			: impl_(v) {}

		/** Convert string in YYYYMMDD format to Date */
		static Date from_iso_string(const char* str) {
			return Date(boost::gregorian::date_from_iso_string(str));
		}

		/** Convert string in YYYYMMDD format to Date */
		static Date from_iso_string(const std::string& str) {
			return Date(boost::gregorian::date_from_iso_string(str));
		}

		/** Convert string in YYYYMMDD format to Date */
		static Date from_string(const char* str) {
			return Date(boost::gregorian::from_simple_string(str));
		}

		/** Convert string in YYYYMMDD format to Date */
		static Date from_string(const std::string& str) {
			return Date(boost::gregorian::from_simple_string(str));
		}

		/** Period in days between two dates 
		@return Period with type == DAYS 
		*/
		Period operator-(Date other) const;

		/** End of current month */
		Date end_of_month() const {
			return impl_.end_of_month();
		}

		/** Return a date iterator which adds the period in each step
		*
		* \period[in] start Initial date
		* \period[in] period Period
		*
		* @return Iterator or null if period.type == PeriodType::NONE
		* @throw std::logic_error If Period type unknown.
		*/
		std::unique_ptr<boost::gregorian::date_iterator> make_date_iterator(const Period& period) const;

		/** Is year a leap year */
		static bool is_leap(year_type year);

		/** Distance between this and other in full lengths of period (i.e. max k so that this + k * period <= other if this <= other; if this > other, return -other.dist(this)).
		@throw std::domain_error if period.size <= 0 or period.type == PeriodType::NONE
		*/
		int dist(Date other, const Period& period) const;

		/** @see dist */
		int dist_years(Date other) const;

		/** @see dist */
		int dist_days(Date other) const;

		bool is_leap() const {
			return is_leap(impl_.year());
		}

		year_type year() const {
			return impl_.year();
		}

		month_type month() const {
			return impl_.month();
		}

		day_type day() const {
			return impl_.day();
		}

		/** Which day of year it is. Return 1 for 1 Jan. */
		uint16_t day_of_year() const {
			return static_cast<uint16_t>(impl_.day_of_year());
		}

		long julian_day() const {
			return impl_.julian_day();
		}

		bool is_special() const {
			return impl_.is_special();
		}

		bool is_not_a_date() const {
			return impl_.is_not_a_date();
		}

		bool is_pos_infinity() const {
			return impl_.is_pos_infinity();
		}

		bool is_neg_infinity() const {
			return impl_.is_neg_infinity();
		}

		bool operator>=(Date other) const {
			return impl_ >= other.impl_;
		}

		bool operator<=(Date other) const {
			return impl_ <= other.impl_;
		}

		bool operator<(Date other) const {
			return impl_ < other.impl_;
		}

		bool operator>(Date other) const {
			return impl_ > other.impl_;
		}

		bool operator==(Date other) const {
			return impl_ == other.impl_;
		}

		bool operator!=(Date other) const {
			return impl_ != other.impl_;
		}

		/** Add a period to a date
		@throw std::logic_error If type is not known.
		@throw std::out_of_range If adding the period would move the date outside supported range
		*/
		friend Date operator+(Date date, const Period& period);

		/** Subtract a period from a date
		@throw std::logic_error If type is not known.
		@throw std::out_of_range If adding the period would move the date outside supported range
		*/
		friend Date operator-(Date date, const Period& period);

		friend std::ostream& operator<<(std::ostream& os, Date date) {
			os << boost::gregorian::to_iso_extended_string(date.impl_);
			return os;
		}

		typedef boost::gregorian::date::ymd_type ymd_type;

		ymd_type year_month_day() const {
			return impl_.year_month_day();
		}
	private:
		boost::gregorian::date impl_;
	};

}

namespace boost {
    namespace gregorian {
		inline size_t hash_value(date const& date)
		{
			return boost::hash_value(date.julian_day());
		}
    }
}

namespace std {
	template <> struct hash<averisera::Date> {
		typedef averisera::Date argument_type;
		typedef size_t result_type;
		result_type operator()(const argument_type& date) const {
			return boost::hash_value(date.julian_day());
		}
	};
}

#endif 
