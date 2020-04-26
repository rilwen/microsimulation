// (C) Averisera Ltd 2014-2020
#include "procreation_calibrator.hpp"
#include "rate_calibrator.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/hazard_curve_factory.hpp"
#include "core/csv_file_reader.hpp"
#include "core/data_exception.hpp"
#include "core/daycount.hpp"
#include "core/generic_distribution_integral.hpp"
#include "core/log.hpp"
#include "core/period.hpp"
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		namespace ProcreationCalibrator {
			const double PREGNANCY_YEAR_FRACTION = 0.75;
			const double FERTILITY_RATE_PERIOD_YEAR_FRACTION = 1.0;

			static std::vector<Conception::mdistr_series_type> calc_multiplicity_distros(DataFrame<age_group_type, int>& multiple_birth_rates, double basis) {
				multiple_birth_rates /= basis;
				const size_t ny = multiple_birth_rates.nbr_rows();
				const size_t ng = multiple_birth_rates.nbr_cols();
				std::vector<Conception::mdistr_series_type> distros(ny);
				for (size_t i = 0; i < ny; ++i) {
					auto& distr_i = distros[i];
					distr_i.reserve(ng);
					for (size_t k = 0; k < ng; ++k) {
						const double start_age = k == 0 ? 0 : multiple_birth_rates.columns()[k].begin();
						std::vector<double> probs(2);
						const double pmult = multiple_birth_rates.ix(i, k); // prob. of multiple birth
						if (pmult < 0 || pmult > 1) {
							throw DataException("ProcreationCalibrator: probability of multiple birth outside [0, 1]");
						}
						probs[1] = pmult;
						probs[0] = 1 - pmult;
						distr_i.push_back(start_age, std::make_shared<GenericDistributionIntegral<Conception::multiplicity_type>>(1, std::move(probs)));
					}
				}
				return distros;
			}

			Conception::mdistr_multi_series_type load_multiplicity_distros(CSVFileReader& reader, double basis) {
				auto values = RateCalibrator::read_values(reader, RateCalibrator::CHECK_AGE_GROUP_OVERLAP | RateCalibrator::PAD_NAN_COLS);
				const std::vector<int> years(values.index()); // copy it just to be safe because calc_multiplicity_distros can modify the dataframe
				return Conception::mdistr_multi_series_type(years, calc_multiplicity_distros(values, basis));
			}

			Conception::mdistr_multi_series_type load_multiplicity_distros(const std::vector<int>& years, CSVFileReader& reader, double basis) {
				auto values = RateCalibrator::read_values_and_interpolate(years, reader, RateCalibrator::CHECK_AGE_GROUP_OVERLAP | RateCalibrator::PAD_NAN_COLS);
				assert(years == values.index());
				return Conception::mdistr_multi_series_type(years, calc_multiplicity_distros(values, basis));
			}

			DataFrame<age_group_type, int> load_birth_numbers(CSVFileReader& reader) {
				DataFrame<age_group_type, int> values(RateCalibrator::read_values(reader, RateCalibrator::CHECK_AGE_GROUP_OVERLAP));
				const auto& age_groups = values.columns();
				if (!age_groups.empty()) {
					auto ag = age_groups.front();
					if (ag.end() <= MIN_CHILDBEARING_AGE) {
						throw DataException("ProcreationCalibrator: mother age group outside bounds");
					}
					if (ag.begin() < MIN_CHILDBEARING_AGE) {
						values.set_column_label(0, age_group_type(MIN_CHILDBEARING_AGE, ag.end()));
					}
				}
				LOG_DEBUG() << "ProcreationCalibrator: loaded birth numbers:\n" << values;
				return values;
			}

			DataFrame<age_group_type, int> load_cohort_birth_rates(CSVFileReader& reader, double basis, bool pad_missing) {
				auto flags = RateCalibrator::CHECK_AGE_GROUP_OVERLAP;
				if (pad_missing) {
					flags = flags | RateCalibrator::USE_NANS_FOR_MISSING | RateCalibrator::PAD_NAN_COLS;
				}
				DataFrame<age_group_type, int> values(RateCalibrator::read_values(reader, flags));
				values /= basis;
				LOG_DEBUG() << "ProcreationCalibrator: loaded cohort birth rates:\n" << values;
				return values;
			}

			DataFrame<age_group_type, int> calculate_conception_hazard_rates(const DataFrame<age_group_type, int>& birth_rates, const Conception::mdistr_multi_series_type& multiplicity_distros, const double post_pregnancy_zero_fertility_year_fraction) {
				DataFrame<age_group_type, int>  hazard_rates(birth_rates.columns(), birth_rates.index());
				typedef DataFrame<age_group_type, int>::size_type size_type;

				// minimum interval between conception events, as year fraction
				const double zero_fertility_year_fraction = PREGNANCY_YEAR_FRACTION + post_pregnancy_zero_fertility_year_fraction;

				for (size_type c = 0; c < birth_rates.nbr_cols(); ++c) {
					for (size_type r = 0; r < birth_rates.nbr_rows(); ++r) {
						const auto age = birth_rates.columns()[c].begin();
						const int year = birth_rates.index()[r] + age;
						const auto& mult_distr_series = multiplicity_distros.padded_value(year);
						const auto* distr_ptr_ptr = mult_distr_series.last_value(age);
						double h;
						if (!distr_ptr_ptr) {
							h = RateCalibrator::hazard_rate_from_average_occurrences(FERTILITY_RATE_PERIOD_YEAR_FRACTION, birth_rates.ix(r, c), zero_fertility_year_fraction);
						} else {
							const std::shared_ptr<const Conception::multiplicity_distr_type> distr_ptr = *distr_ptr_ptr;
							assert(distr_ptr != nullptr);
							h = RateCalibrator::hazard_rate_from_average_increment<Conception::multiplicity_type>(FERTILITY_RATE_PERIOD_YEAR_FRACTION, birth_rates.ix(r, c), zero_fertility_year_fraction, *distr_ptr);
						}						
						hazard_rates.ix(r, c) = h;
					}
				}
				LOG_DEBUG() << "ProcreationCalibrator: calculated conception hazard rates:\n" << hazard_rates;
				return hazard_rates;
			}

			std::vector<std::unique_ptr<AnchoredHazardCurve>> calculate_conception_hazard_curves(const DataFrame<age_group_type, int>& conception_hazard_rates) {
				std::vector<std::unique_ptr<AnchoredHazardCurve>> curves(conception_hazard_rates.nbr_rows());
				static const std::shared_ptr<const HazardCurveFactory> hazard_curve_factory = HazardCurveFactory::PIECEWISE_CONSTANT();
				static const std::shared_ptr<const Daycount> daycount = Daycount::DAYS_365_25(); // YEAR_FRACT(); // YEAR_FRACT is slower
				static const bool conditional = true;
				static const bool periods_additive = true;
				for (size_t i = 0; i < conception_hazard_rates.nbr_rows(); ++i) {
					const auto yob = conception_hazard_rates.index()[i];
					const Date start(MathUtils::safe_cast<Date::year_type>(yob), 1, 1);
					std::vector<double> jump_probs;
					std::vector<Period> periods;
					jump_probs.reserve(conception_hazard_rates.nbr_cols());
					periods.reserve(conception_hazard_rates.nbr_cols());
					double h = 0.0;
					Date prev_date = start;
					age_type prev_age = 0;
					const auto age_to_date = [yob](age_type age) {
						// take into account that the data are are gathered over 1 year and registered for the year of giving birth, not the one of conception
						return Date(MathUtils::safe_cast<Date::year_type>(yob + age), 6, 1) - Period::months(9);
					};
					for (size_t j = 0; j < conception_hazard_rates.nbr_cols(); ++j) {
						const auto age = conception_hazard_rates.columns()[j].begin();
						assert(age > 0);						
						const Date date = age_to_date(age);
						const double dt = daycount->calc(prev_date, date);
						const double jump_probability = HazardCurve::jump_probability(h *dt);
						jump_probs.push_back(jump_probability);
						if (j > 0) {
							periods.push_back(Period::years(date.year() - prev_date.year()));
						} else {
							periods.push_back(date - prev_date);
						}
						h = conception_hazard_rates.ix(i, j);
						prev_date = date;
						prev_age = age;
					}
					// last one
					if (MAX_CHILDBEARING_AGE > prev_age) {
						const double dh = h / static_cast<double>(MAX_CHILDBEARING_AGE - prev_age);
						for (age_type age = prev_age + 1; age <= MAX_CHILDBEARING_AGE; ++age) {
							const Date date = age_to_date(age);
							const double dt = daycount->calc(prev_date, date);
							jump_probs.push_back(HazardCurve::jump_probability(h * dt));
							periods.push_back(Period::years(age - prev_age));
							prev_date = date;
							prev_age = age;
							h = std::max(0.0, h - dh);
						}
						assert(std::abs(h) < 1e-6 * dh);
					}
					jump_probs.push_back(HazardCurve::jump_probability(0.0));
					periods.push_back(Period::years(1));
					LOG_DEBUG() << "ProcreationCalibrator: conception hazard curve for YOB " << yob << ": jump probabilities: " << jump_probs << ", periods: " << periods;
					curves[i] = AnchoredHazardCurve::build(start, daycount, hazard_curve_factory, periods, jump_probs, periods_additive, conditional, std::vector<HazardRateMultiplier>());
				}
				return curves;
			}

			DataFrame<Sex, int> load_gender_rates(CSVFileReader& reader, const double basis) {
				check_that<DataException>(basis >= 0, "Basis cannot be negative");
				DataFrame<Sex, int> rates(DataFrame<Sex, int>::from_csv_file(reader, [](const std::string& elem) { return sex_from_string(elem);  }, reader.default_converter_complaining<int>(DataFrame<Sex, int>::INDEX_COLUMN_POSITION), false, false));
				rates.sort_index();
				if (rates.nbr_cols() != 2) {
					throw DataException(boost::str(boost::format("ProcreationCalibrator: expected 2 columns in file %s") % reader.file_name()));
				}
				if (rates.nbr_rows() == 0) {
					throw DataException(boost::str(boost::format("ProcreationCalibrator: expected at least 1 row in file %s") % reader.file_name()));
				}
				for (size_t r = 0; r < rates.nbr_rows(); ++r) {
					for (size_t c = 0; c < 2; ++c) {
						if (rates.ix(r, c) < 0) {
							throw DataException(boost::str(boost::format("ProcreationCalibrator: negative value %g in row %d, column %d in file %s") % rates.ix(r, c) % r % c % reader.file_name()));
						}
					}
					if (rates.row_values_ix(r).norm() == 0) {
						const double filler = basis > 0 ? basis / 2 : 1;
						for (size_t c = 0; c < 2; ++c) {
							rates.ix(r, c) = filler;
						}					
					}
				}
				if (basis > 0) {
					rates /= basis;					
				} else {
					const auto sumcols = rates.col_values_ix(0) + rates.col_values_ix(1);
					Eigen::MatrixXd values(rates.values());
					values.col(0).array() /= sumcols.array();
					values.col(1).array() /= sumcols.array();
					rates = DataFrame<Sex, int>(values, rates.columns(), rates.index());
				}
				LOG_DEBUG() << "ProcreationCalibrator: loaded gender rates:\n" << rates;
				return rates;
			}

			DataFrame<Ethnicity::index_range_type, NumericalRange<int>> load_total_fertility_rates_for_ethnic_groups(CSVFileReader& reader, const Ethnicity::IndexConversions& ic) {
				typedef DataFrame<Ethnicity::index_range_type, NumericalRange<int>> df_type;
				const auto group_range_converter = [&ic](const std::string& str) {
					return ic.index_range_from_string(str.c_str());
				};
				const auto year_range_converter = [](const std::string& str) {
					return NumericalRange<int>::from_string_open_ended(str.c_str(), boost::lexical_cast<int, std::string>, nullptr, nullptr);
				};
				df_type result(df_type::from_csv_file(reader, group_range_converter, year_range_converter, false, false));
				result.sort_index();
				LOG_DEBUG() << "ProcreationCalibrator: loaded total fertility rates for ethnic groups:\n" << result;
				return result;
			}
		}
	}
}
