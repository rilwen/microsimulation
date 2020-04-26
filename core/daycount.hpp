// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_DAYCOUNT_H
#define __AVERISERA_DAYCOUNT_H

#include "dates_fwd.hpp"
#include <iosfwd>
#include <memory>

namespace averisera {
    /** Calculates the year fraction between two dates */
    class Daycount {
    public:
        virtual ~Daycount();

        /** Calculate the year fraction between 0:00 AM of Date d1 and 00:00 AM of Date d1.
          Returns 0 if and only if d1 == d2. If d1 < d2, then calc(d1, d2) < 0.
          calc(d1, d2) = - calc(d2, d1). If d1 < d2, then for any d calc(d, d1) < calc(d, d2) and
          calc(d, d1) == calc(d, d2) if and only if d1 == d2.
          @param d1 Valid date
          @param d2 Valid date
        */
        virtual double calc(Date d1, Date d2) const = 0;

        /** (For yfr >= 0) Find latest date d2 such that calc(d1, d2) is nearest yfr.
          (For yfr < 0) Find earliest date d2 such that calc(d1, d2) is nearest yfr.
          @param d1 Valid date
          @throw std::out_of_range If yfr is finite but so large that d1 + yfr would be outside the supported date range
         */
        virtual Date add_year_fraction(Date d1, double yfr) const = 0;

        virtual void print(std::ostream& s) const = 0;

        /** (julian(d2) - julian(d1)) / 365 */
        static std::shared_ptr<const Daycount> DAYS_365();

        /** (julian(d2) - julian(d1)) / 365.25 */
        static std::shared_ptr<const Daycount> DAYS_365_25();

        /** Let y1 := year(d1) and y2 := year(d2).
          Let basis(year) = 366 if year is a leap year, 365 otherwise.
          
          Return max(y2 - y1 - 1, 0) + (julian(y1 + 1, January, 1) - julian(d1)) / basis(y1) + (julian(d2) - julian(y2, January, 1)) / basis(y2)
         */
        static std::shared_ptr<const Daycount> YEAR_FRACT();

		/** @throw std::runtime_error If str can't be parsed */
		static std::shared_ptr<const Daycount> from_string(const char* str);
    };

    inline std::ostream& operator<<(std::ostream& out, const Daycount& daycount) {
        daycount.print(out);
        return out;
    }


}

#endif // __AVERISERA_DAYCOUNT_H
