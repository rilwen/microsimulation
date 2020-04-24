#include "daycount.hpp"
#include "dates.hpp"
#include "math_utils.hpp"
#include "period.hpp"
#include "preconditions.hpp"
#include "stl_utils.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
    Daycount::~Daycount() {
    }

    /** Difference in days divided by constant basis */
    class DaysOverBasis: public Daycount {
    public:
        DaysOverBasis(double basis)
            : _basis(basis) {
            assert(_basis > 0);
        }

        double calc(Date d1, Date d2) const override {
            assert(!d1.is_special());
            assert(!d2.is_special());
            return static_cast<double>((d2 - d1).days()) / _basis;
        }

        Date add_year_fraction(Date d1, double yfr) const override {
            assert(!d1.is_special());
            if (std::isinf(yfr)) {
                return yfr > 0 ? Date::POS_INF : Date::NEG_INF;
            }
            try {
                const int duration_days = MathUtils::safe_cast<int>(round(yfr * _basis));
                return d1 + Period(PeriodType::DAYS, duration_days);
            } catch (std::out_of_range& e) {
                throw std::out_of_range(boost::str(boost::format("Daycount::DAYS_%g: date range exceeded when adding year fraction %g to date %s: %s") % _basis % yfr % boost::lexical_cast<std::string>(d1) % e.what()));
            }
        }

        void print(std::ostream& s) const override {
            s << "DAYS_" << _basis;
        }
    private:
        double _basis;
    };

    /** Fraction of years between dates */
    class YearFraction: public Daycount {
    public:
        double calc(Date d1, Date d2) const override {
            assert(!d1.is_special());
            assert(!d2.is_special());
            if (d2 >= d1) {               
                const auto y1 = d1.year();
                const auto y2 = d2.year();
                if (y2 > y1) {
                    const long whole_part = std::max(y2 - y1 - 1, 0);
					const double head_fract = 1.0 - static_cast<double>(d1.day_of_year() - 1) / basis_for_year(y1);
                    const double tail_fract = static_cast<double>(d2.day_of_year() - 1) / basis_for_year(y2);
                    return static_cast<double>(whole_part) + (head_fract + tail_fract);
                } else {
                    return static_cast<double>((d2 - d1).days()) / basis_for_year(y1);
                }
            } else {
                return - calc(d2, d1);
            }
        }

        Date add_year_fraction(Date d1, double yfr) const override {
            constexpr double eps = 1E-13;
            constexpr double eps_plus_1 = 1 + eps;
            assert(!d1.is_special());
            if (std::isinf(yfr)) {
                return yfr > 0 ? Date::POS_INF : Date::NEG_INF;
            }
            if (yfr >= 0) {
                if (yfr > calc(d1, Date::MAX) * eps_plus_1) {
                    throw std::out_of_range(boost::str(boost::format("Daycount: year fraction %g added moves date %s out of range") % yfr % d1));
                }
                while (yfr > eps) {
                    const auto y1 = d1.year();
                    Date next_d;
                    try {
						if (y1 < Date::MAX_YEAR) {
							next_d = Date(static_cast<Date::year_type>(y1 + 1), 1, 1);
						} else {
							next_d = Date::MAX;
						}
                    } catch (std::exception& e) {
                        // just in case the first test didn't catch this
						throw std::out_of_range(boost::str(boost::format("Daycount: year fraction %g added moves date %s out of range: %s") % yfr % d1 % e.what()));                        
                    }
                    const double basis = basis_for_year(y1);
                    const double fract = static_cast<double>((next_d - d1).days()) / basis;
                    assert(fract >= 0);
                    if (fract >= yfr || d1 == Date::MAX) {
                        // it ends this year
                        return d1 + Period(PeriodType::DAYS, static_cast<int>(round(yfr * basis)));
                    } else {
                        d1 = next_d;
                        yfr = std::max(0., yfr - fract);						
                    }
                }
            } else {
                const double from_min_to_d1 = calc(d1, Date::MIN);
                if (yfr < from_min_to_d1 * eps_plus_1) {
                    //std::cout << from_min_to_d1 << std::endl;
                    throw std::out_of_range(boost::str(boost::format("Daycount: adding year fraction %g moves date out of range") % yfr));
                }
                while (yfr < 0) {                    
                    //std::cout << d1 << " " << yfr << std::endl;
                    auto y1 = d1.year();
                    Date next_d = Date(y1, 1, 1);
                    assert(next_d >= Date::MIN);
                    if (next_d == d1) {
                        y1 = static_cast<Date::year_type>(y1 - 1);
                        next_d = Date(y1, 1, 1);
                    }
                    const double basis = basis_for_year(y1);
                    const double fract = static_cast<double>((next_d - d1).days()) / basis;
                    assert(fract <= 0);
                    if (yfr >= fract) {
                        // it ends this year
                        return d1 + Period(PeriodType::DAYS, static_cast<int>(round(yfr * basis)));
                    } else {
                        d1 = next_d;
                        yfr = std::min(0., yfr - fract);
                    }
                }
            }
            return d1;
        }

        void print(std::ostream& s) const override {
            s << "YEAR_FRACT";
        }
    private:
		inline static double basis_for_year(Date::year_type year) {
            return Date::is_leap(year) ? 366.0 : 365.0;
        }
    };

	/** Caches results of calc() */
	class CachingDaycount : public Daycount {
	public:
		CachingDaycount(std::shared_ptr<const Daycount> impl)
			: impl_(impl) {
			check_not_null(impl, "CachingDaycount: null implementation");
		}

		double calc(Date d1, Date d2) const override {
			const std::pair<Date, Date> key(d1, d2);
			const auto it = yfr_cache_.find(key);
			if (it != yfr_cache_.end()) {
				return it->second;
			} else {
				const double yfr = impl_->calc(d1, d2);
				yfr_cache_.insert(std::make_pair(key, yfr));
				return yfr;
			}
		}

		inline Date add_year_fraction(Date d1, double yfr) const override {
			return impl_->add_year_fraction(d1, yfr);
		}

		inline void print(std::ostream& s) const override {
			impl_->print(s);
		}
	private:
		mutable std::unordered_map<std::pair<Date, Date>, double> yfr_cache_;
		std::shared_ptr<const Daycount> impl_;
	};

    std::shared_ptr<const Daycount> Daycount::DAYS_365() {
        const static auto dcc(std::make_shared<DaysOverBasis>(365.0));
        return dcc;
    }

    std::shared_ptr<const Daycount> Daycount::DAYS_365_25() {
        const static auto dcc(std::make_shared<DaysOverBasis>(365.25));
        return dcc;
    }

    std::shared_ptr<const Daycount> Daycount::YEAR_FRACT() {
        const static auto dcc(std::make_shared<YearFraction>());
        return dcc;
    }

	std::shared_ptr<const Daycount> Daycount::from_string(const char* str) {
		if (!strcmp(str, "DAYS_365")) {
			return DAYS_365();
		} else if (!strcmp(str, "DAYS_365.25")) {
			return DAYS_365_25();
		} else if (!strcmp(str, "YEAR_FRACT")) {
			return YEAR_FRACT();
		} else {
			throw std::runtime_error(boost::str(boost::format("Daycount: cannot parse string <%s>") % str));
		}
	}
}
