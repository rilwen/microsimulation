// (C) Averisera Ltd 2014-2020
#include "migration_model.hpp"
#include "core/brent.hpp"
#include "core/daycount.hpp"
#include "core/log.hpp"
#include "core/preconditions.hpp"
#include <ostream>

namespace averisera {
	namespace microsim {
		MigrationModel::MigrationRatePerAnnum::MigrationRatePerAnnum(const double rel, const double abs)
			: relative_(rel), absolute_(abs) {			
		}

		static double calc_migration_impl(const double x0, const double t, const double rel, const double abs) {
			if (rel == 0) {
				return t * abs;
			} else {
				const double y0 = x0 + abs / rel;
				return y0 * expm1(rel * t);
			}
		}

		double MigrationModel::calculate_migration(const double x0, const double t, const MigrationRatePerAnnum& r) {
			return calc_migration_impl(x0, t, r.relative(), r.absolute());
		}

		double MigrationModel::calculate_migration(Date from, Date to, double x0) const {
			const auto rate = get_rate(from);
			LOG_TRACE() << "MigrationModel: using rate " << rate;
			const double dt = calc_dt(from, to);
			return calculate_migration(x0, dt, rate);
		}

		MigrationModel::MigrationRatePerAnnum MigrationModel::calibrate_rate(const double x0, const double xt, const double t, const double absolute_rate) {
			check_that<std::out_of_range>(x0 >= 0, "MigrationModel: initial population must be positive or zero");
			check_that<std::out_of_range>(xt >= 0, "MigrationModel: final population must be positive or zero");
			check_that<std::out_of_range>(t > 0, "MigrationModel: time interval must be positive");
			check_that<std::out_of_range>(x0 != 0 || xt == 0, "MigrationModel: initial population must be non-zero unless the final population is also zero");
			if (x0 == 0) {
				assert(xt == 0);
				return MigrationRatePerAnnum(-std::numeric_limits<double>::infinity(), absolute_rate);
			}
			const double rel0 = log(xt / x0) / t;
			if (absolute_rate == 0) {
				return MigrationRatePerAnnum(rel0, absolute_rate);
			} else {
				double rel_lo = rel0;
				double rel_hi = rel0;
				const double da = std::abs(absolute_rate) / x0;
				const auto fun = [x0, xt, t, absolute_rate](double rel) { return x0 + calc_migration_impl(x0, t, rel, absolute_rate) - xt; };
				while (fun(rel_hi) < 0) {
					rel_hi += da;
				}
				while (fun(rel_lo) > 0) {
					rel_lo -= da;
				}
				const double rel = RootFinding::find_root(fun, rel_lo, rel_hi, 1e-8 * rel0, 1e-8 * x0);
				return MigrationRatePerAnnum(rel, absolute_rate);
			}
		}

		MigrationModel::MigrationModel()
		{}

		MigrationModel::MigrationModel(time_dependent_migration_rate&& migration_rates) {
			migration_rates_ = std::move(migration_rates);
		}

		MigrationModel::MigrationModel(MigrationRatePerAnnum rate)
			: MigrationModel(time_dependent_migration_rate(Date(2000, 1, 1), rate))
		{}

		MigrationModel::MigrationModel(MigrationModel&& other)
			: migration_rates_(std::move(other.migration_rates_))
		{}

		MigrationModel& MigrationModel::operator=(MigrationModel&& other) {
			migration_rates_ = std::move(other.migration_rates_);
			return *this;
		}

		MigrationModel::MigrationRatePerAnnum MigrationModel::get_rate(const Date date) const {
			if (!migration_rates_.empty()) {
				return migration_rates_.padded_value(date);				
			}
			return MigrationRatePerAnnum();
		}

		double MigrationModel::calc_dt(Date from, Date to) {
			return Daycount::YEAR_FRACT()->calc(from, to);
		}

		std::ostream& operator<<(std::ostream& os, const MigrationModel::MigrationRatePerAnnum& rate) {
			os << "(ABS=" << rate.absolute() << ", REL=" << rate.relative() << ")";
			return os;
		}
	}
}
