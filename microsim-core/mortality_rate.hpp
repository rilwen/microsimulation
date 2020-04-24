#pragma once
#include "core/dates.hpp"
#include <iosfwd>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		/*! Mortality rate for a group 
		\tparam G Value identifying a group */
		template <class G> struct MortalityRate {
			typedef Date::year_type year_t;
			/*! Mortality rate for given year 
			\param n_rate What fraction of the group will die in [n_from, n_to) period.
			\throw std::out_of_range If n_rate < 0 or n_year outside of [Date::MIN_YEAR, Date::MAX_YEAR] range.  */
			MortalityRate(Date::year_type n_year, double n_rate, G n_group);

			template <class F> MortalityRate(const MortalityRate<F>& other, G new_group)
				: MortalityRate(other.year, other.rate, new_group) {}

			/*! G must be assignable from F */
			template <class F> MortalityRate(const MortalityRate<F>& other)
				: MortalityRate(other.year, other.rate, other.group) {}

			bool operator<(const MortalityRate<G>& other) const {
				if (year != other.year) {
					return year < other.year;
				} else {
					return group < other.group;
				}
			}

			Date::year_type year; /*! Year with mortality rate */
			double rate; /*!< Mortality rate (as fraction) */
			G group; /*! Group (e.g. age group)*/
		};

		template <class G> MortalityRate<G>::MortalityRate(year_t n_year, double n_rate, G n_group)
			: year(n_year), rate(n_rate), group(n_group) {
			if (rate < 0) {
				throw std::out_of_range(boost::str(boost::format("MortalityRate: negative rate %g") % rate));
			}
			if (year < Date::MIN_YEAR || year > Date::MAX_YEAR) {
				throw std::out_of_range(boost::str(boost::format("MortalityRate: year %d outside allowed range [%d, %d]") % year % Date::MIN_YEAR % Date::MAX_YEAR));
			}
		}

		template <class G> std::ostream& operator<<(std::ostream& os, const MortalityRate<G>& mr) {
			os << "MR(" << mr.rate << ", " << mr.year << ", " << mr.group << ")";
			return os;
		}
	}
}
