#include "mortality_calibrator.hpp"
#include "rate_calibrator.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/hazard_curve_factory.hpp"
#include "microsim-core/mortality_rate.hpp"
#include "core/dates.hpp"
#include "core/daycount.hpp"
#include "core/exceptions.hpp"
#include "core/math_utils.hpp"
#include "core/period.hpp"
#include "core/stl_utils.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
	namespace microsim {
		namespace MortalityCalibrator {
			typedef MortalityRate<Date::year_type> cohort_mortality_rate_t; // mortality rate for a cohort with given year of birth
			static const auto MORTALITY_DAYCOUNT = Daycount::DAYS_365(); // you can die any time!

			static double extrapolate_rate(const double rate, const Date start_date_src, const Date end_date_src, 
				const Date start_date_dst, const Date end_date_dst) {
				const double dt_src = MORTALITY_DAYCOUNT->calc(start_date_src, end_date_src);
				const double dt_dst = MORTALITY_DAYCOUNT->calc(start_date_dst, end_date_dst);
				assert(dt_src > 0);
				assert(dt_dst > 0);
				return HazardCurve::extrapolate_proba(rate, dt_dst / dt_src);
			}

			// horizon_years_src: to start of rate
			// horizon_years_dst: to start of period we want to cover with this rate
			// next_horizon_years: to start of next rate
			static double extrapolate_rate(const double rate, const Date cohort_date_of_birth, const int horizon_years_src, 
				const int horizon_years_dst, const int next_horizon_years) {
				const Date d1_src = cohort_date_of_birth + Period::years(horizon_years_src);
				const Date d1_dst = cohort_date_of_birth + Period::years(horizon_years_dst);
				const Date d2_dst = cohort_date_of_birth + Period::years(next_horizon_years);
				return extrapolate_rate(rate, d1_src, d1_src + Period::years(1), d1_dst, d2_dst);
			}

			static inline double move_rate(const double rate, Date::year_type yr_src, Date::year_type yr_dst) {
				return extrapolate_rate(rate, Date(yr_src, 1, 1), Date(static_cast<Date::year_type>(yr_src + 1), 1, 1), Date(yr_dst, 1, 1), Date(static_cast<Date::year_type>(yr_dst + 1), 1, 1));
			}

			/*!
			All rates in mortality_rates should have the same group == year of birth
			*/
			static std::unique_ptr<AnchoredHazardCurve> from_mortality_rates(std::shared_ptr<const Daycount> daycount, const std::vector<cohort_mortality_rate_t>& mortality_rates) {
				assert(!mortality_rates.empty());
				static const bool periods_additive = false;
				static const bool conditional = true;
				static const std::shared_ptr<const HazardCurveFactory> hazard_curve_factory = HazardCurveFactory::PIECEWISE_CONSTANT();
				std::vector<double> jump_probs;
				std::vector<Period> periods;				
				const size_t n = mortality_rates.size();
				jump_probs.reserve(n);
				periods.reserve(n);
				const Date cohort_date_of_birth(mortality_rates.front().group, 1, 1);
				const auto calc_horizon_years = [](const cohort_mortality_rate_t& mr) { return mr.year - mr.group; }; // horizon in years to the BEGINNING of the period covered by this rate
				const auto calc_horizon_days = [cohort_date_of_birth](int years) { return (cohort_date_of_birth + Period::years(years)) - cohort_date_of_birth; };

				int prev_horizon_years_src = calc_horizon_years(mortality_rates.front());
				int prev_horizon_years_dst = 0;
				for (size_t i = 1; i < n; ++i) {
					const auto& mr = mortality_rates[i];
					const int horizon_years = calc_horizon_years(mr);
					const double jp = extrapolate_rate(mortality_rates[i - 1].rate, cohort_date_of_birth, prev_horizon_years_src, prev_horizon_years_dst, horizon_years);
					jump_probs.push_back(jp);					
					periods.push_back(calc_horizon_days(horizon_years));
					prev_horizon_years_src = horizon_years;
					prev_horizon_years_dst = horizon_years;
				}
				const int last_horizon_years = prev_horizon_years_dst + 1;
				periods.push_back(calc_horizon_days(last_horizon_years));
				jump_probs.push_back(extrapolate_rate(mortality_rates.back().rate, cohort_date_of_birth, prev_horizon_years_src, prev_horizon_years_dst, last_horizon_years));
				assert(periods.front().size > 0);
				return AnchoredHazardCurve::build(cohort_date_of_birth, MORTALITY_DAYCOUNT, hazard_curve_factory, periods, jump_probs, periods_additive, conditional, std::vector<HazardRateMultiplier>());
			}

			std::vector<std::unique_ptr<AnchoredHazardCurve>> calc_mortality_curves(const std::vector<MortalityRate<age_group_type>>& mortality_rates, Date::year_type min_year_of_birth, Date::year_type max_year_of_birth) {
				if (min_year_of_birth > max_year_of_birth) {
					throw std::domain_error("MortalityCalibrator: min year of birth larger than max year of birth");
				}
				min_year_of_birth = std::max(min_year_of_birth, Date::MIN_YEAR);
				max_year_of_birth = std::min(max_year_of_birth, Date::MAX_YEAR);
				
				const size_t nbr_curves = max_year_of_birth - min_year_of_birth + 1;
				std::vector<std::vector<cohort_mortality_rate_t>> cohort_mortality_rates(nbr_curves);
				for (const auto& mr : mortality_rates) {
					// apply the (year, age group) mortality rate to every  (year, year of birth) tuple possible

					// maximum year of birth
					Date::year_type max_yr = static_cast<Date::year_type>(std::min<unsigned int>(max_year_of_birth, mr.year - mr.group.begin()));
					// minimum year of birth
					Date::year_type min_yr = static_cast<Date::year_type>(std::max<unsigned int>(min_year_of_birth, mr.year - mr.group.end() + 1));					
					if (min_yr < min_year_of_birth) {
						min_yr = min_year_of_birth;
					}
					if (max_yr > max_year_of_birth) {
						max_yr = max_year_of_birth;
					}
					for (Date::year_type yr = min_yr; yr <= max_yr; ++yr) {
						// yr = year of birth
						cohort_mortality_rates[yr - min_year_of_birth].push_back(cohort_mortality_rate_t(mr, yr));
					}
				}

				std::vector<std::unique_ptr<AnchoredHazardCurve>> curves;
				curves.reserve(nbr_curves);
				size_t first_nonnull_curve_idx = nbr_curves;
				size_t one_past_last_nonnull_curve_idx = 0;
				
				for (size_t i = 0; i < nbr_curves; ++i) {
					auto& v = cohort_mortality_rates[i];
					if (!v.empty()) {
						first_nonnull_curve_idx = std::min(first_nonnull_curve_idx, i);
						one_past_last_nonnull_curve_idx = i + 1;
						std::sort(v.begin(), v.end());
						for (auto it = v.begin() + 1; it != v.end(); ++it) {
							if (it->year == (it - 1)->year) {
								throw DataException(boost::str(boost::format("MortalityCalibrator: duplicate data for year %d and cohort %d") % it->year % it->group));
							}
						}
						LOG_DEBUG() << boost::str(boost::format("MortalityCalibrator: mortality rates for cohort %d: ") % v.front().group) << v;
						curves.push_back(from_mortality_rates(MORTALITY_DAYCOUNT, v));
					} else {
						if (one_past_last_nonnull_curve_idx > 0) {
							LOG_DEBUG() << "MortalityCalibrator: cloning curve for cohort " << (min_year_of_birth + i) << " from cohort " << curves[one_past_last_nonnull_curve_idx - 1]->start().year();
							curves.push_back(curves[one_past_last_nonnull_curve_idx - 1]->move(Date(static_cast<Date::year_type>(min_year_of_birth + i), 1, 1)));
						} else {
							curves.push_back(nullptr);
						}
					}
				}
				
				LOG_INFO() << "MortalityCalibrator: need to backfill " << first_nonnull_curve_idx << " mortality curves";
				// backfill
				for (size_t i = 0; i < first_nonnull_curve_idx; ++i) {
					curves[i] = curves[first_nonnull_curve_idx]->move(Date(static_cast<Date::year_type>(min_year_of_birth + i), 1, 1));
				}
				
				return curves;
			}

			static age_group_type check_age_groups(std::vector<age_group_type> age_groups) {
				assert(!age_groups.empty());
				std::sort(age_groups.begin(), age_groups.end());
				for (auto it = age_groups.begin() + 1; it != age_groups.end(); ++it) {
					if (!it->is_disjoint_with(*(it-1))) {
						throw DataException(boost::str(boost::format("MortalityCalibrator: overlapping age groups %s and %s") % *(it - 1) % *it));
					}
				}
				return age_group_type(age_groups.front().begin(), age_groups.back().end());
			}

			std::vector<std::unique_ptr<AnchoredHazardCurve>> calc_mortality_curves(const DataFrame<age_group_type, int, double>& rates_df, Date::year_type min_year_of_birth, Date::year_type max_year_of_birth) {
				const Eigen::MatrixXd& rates = rates_df.values();
				const auto& years = rates_df.index();
				const auto& age_groups = rates_df.columns();
				const size_t nyears = years.size();
				const size_t ngrps = age_groups.size();
				if (nyears == 0 || nyears == 0) {
					throw std::domain_error("MortalityCalibrator: no data");
				}
				if (static_cast<size_t>(rates.cols()) != ngrps || static_cast<size_t>(rates.rows()) != nyears) {
					throw std::domain_error("MortalityCalibrator: rates matrix has wrong dimensions");
				}
				check_age_groups(age_groups);
                const age_group_type last_age_group = *std::max_element(age_groups.begin(), age_groups.end());
				std::vector<MortalityRate<age_group_type>> mortality_rates;
				mortality_rates.reserve((max_year_of_birth + last_age_group.begin()- min_year_of_birth) * ngrps);
				for (size_t c = 0; c < ngrps; ++c) {
					const auto& age_group = age_groups[c];

					// apply the first rate to cohorts which were born too early to have the mortality rate available for this age group
					auto src_yr = static_cast<Date::year_type>(years[0]);
					for (Date::year_type yr = min_year_of_birth; yr < years.front(); ++yr) {
						mortality_rates.push_back(MortalityRate<age_group_type>(yr, move_rate(rates(0, c), src_yr, yr), age_group));
					}

					// assign the mortality rates to their cohorts (YOB = YEAR - AGE)
					for (size_t r = 0; r < nyears; ++r) {
						mortality_rates.push_back(MortalityRate<age_group_type>(MathUtils::safe_cast<MortalityRate<age_group_type>::year_t>(years[r]), rates(r, c), age_group));
					}

					// apply the last mortality rate to cohorts who were born too late
					src_yr = static_cast<Date::year_type>(years[nyears - 1]);
					for (Date::year_type yr = static_cast<Date::year_type>(years.back() + 1); yr <= max_year_of_birth + last_age_group.begin(); ++yr) {
						mortality_rates.push_back(MortalityRate<age_group_type>(yr, move_rate(rates(nyears - 1, c), src_yr, yr), age_group));
					}
				}
				std::sort(mortality_rates.begin(), mortality_rates.end());
				return calc_mortality_curves(mortality_rates, min_year_of_birth, max_year_of_birth);
 			}

			std::vector<std::unique_ptr<AnchoredHazardCurve>> calc_mortality_curves(CSVFileReader& reader, Date::year_type min_year, Date::year_type max_year) {
				return calc_mortality_curves(RateCalibrator::read_values(reader, RateCalibrator::CHECK_AGE_GROUP_OVERLAP), min_year, max_year);
			}		

			std::vector<std::unique_ptr<AnchoredHazardCurve>> calc_mortality_curves(CSVFileReader& death_reader, CSVFileReader& group_size_reader, Date::year_type min_year_of_birth, Date::year_type max_year_of_birth) {
				const DataFrame<age_group_type, int> rates(RateCalibrator::read_and_calculate_rates(death_reader, group_size_reader, RateCalibrator::CHECK_AGE_GROUP_OVERLAP));
				const auto& years = rates.index();
				if (rates.nbr_rows() > 1) {
					auto prev_it = years.begin();
					for (auto next_it = prev_it + 1; next_it != years.end(); ++prev_it, ++next_it) {
						if (*next_it != *prev_it + 1) {
							throw DataException("MortalityCalibrator: years are not consecutive");
						}
					}
				}
				return calc_mortality_curves(rates, min_year_of_birth, max_year_of_birth);
			}
		}
	}
}
