#include "rate_calibrator.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "core/brent.hpp"
#include "core/distribution.hpp"
#include "core/csv_file_reader.hpp"
#include "core/exceptions.hpp"
#include "core/interpolator.hpp"
#include "core/preconditions.hpp"
#include "core/rng_impl.hpp"
#include "core/running_statistics.hpp"
#include <algorithm>
#include <cassert>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		namespace RateCalibrator {
			Eigen::MatrixXd calculate_rates(const Eigen::MatrixXd& occurrence_counts, const Eigen::MatrixXd& group_sizes) {
				if (occurrence_counts.rows() != group_sizes.rows()) {
					throw std::domain_error("RateCalibrator: numbers of rows are different");
				}
				if (occurrence_counts.cols() != group_sizes.cols()) {
					throw std::domain_error("RateCalibrator: numbers of columns are different");
				}
				Eigen::MatrixXd rates(occurrence_counts.rows(), occurrence_counts.cols());
				typedef decltype(occurrence_counts.rows()) idx_t;
				for (idx_t r = 0; r < rates.rows(); ++r) {
					for (idx_t c = 0; c < rates.cols(); ++c) {
						const double oc = occurrence_counts(r, c);
						if (oc < 0) {
							throw DataException("RateCalibrator: negative occurrence count");
						}
						const double gs = group_sizes(r, c);
						if (gs < 0) {
							throw DataException("RateCalibrator: negative group size");
						}
						if (oc > gs) {
							throw DataException("RateCalibrator: occurrence count larger than group size");
						}
						if (oc == 0 && gs == 0) {
							rates(r, c) = 0.0;
						} else {
							rates(r, c) = oc / gs;
						}
					}
				}
				return rates;
			}

			DataFrame<age_group_type, int, double> read_and_calculate_rates(CSVFileReader& occurrence_reader, CSVFileReader& group_size_reader, flag_type flags) {
				DataFrame<age_group_type, int, double> occurrences(read_values(occurrence_reader, flags));
				DataFrame<age_group_type, int, double> group_sizes(read_values(group_size_reader, flags));
				if (occurrences.index() != group_sizes.index()) {
					throw DataException("RateCalibrator: different year vectors");
				}			
				if (occurrences.columns() != group_sizes.columns()) {
					if (flags & AGGREGATE_TO_COARSER_AGE_GROUP) {
						if (group_sizes.nbr_cols() < occurrences.nbr_cols()) {
							occurrences = aggregate_age_groups(occurrences, group_sizes.columns());							
						} else {
							group_sizes = aggregate_age_groups(group_sizes, occurrences.columns());
						}
					} else {
						throw DataException("RateCalibrator: different age group vectors");
					}
				}
				return calculate_rates<age_group_type, int>(occurrences, group_sizes);
			}

			DataFrame<age_group_type, int, double> aggregate_and_calculate_rates(const DataFrame<age_group_type, int, double>& occurrences, const DataFrame<age_group_type, int, double>& group_sizes) {
				if (occurrences.columns() != group_sizes.columns()) {
					if (group_sizes.nbr_cols() < occurrences.nbr_cols()) {
						DataFrame<age_group_type, int, double> no(aggregate_age_groups(occurrences, group_sizes.columns()));
						return calculate_rates<age_group_type, int>(no, group_sizes);
					} else {
						DataFrame<age_group_type, int, double> ng(aggregate_age_groups(group_sizes, occurrences.columns()));
						return calculate_rates<age_group_type, int>(occurrences, ng);
					}
				}
				return calculate_rates(occurrences, group_sizes);
			}

			const std::string& YEAR_COLUMN_NAME() {
				static const std::string value("YEAR");
				return value;
			}

			const size_t YEAR_COLUMN_INDEX = 0;

			age_group_type age_group_from_string(const std::string& orig_str) {
				return age_group_type::from_string_open_ended(orig_str.c_str(), [](const std::string& str) { return boost::lexical_cast<age_type>(str); }, nullptr, &RateCalibrator::MAX_AGE);
				/*std::vector<std::string> ages;
				std::string str(orig_str);
				boost::erase_all(str, " ");
				boost::split(ages, str, boost::is_any_of("-"));
				age_t min_age;
				age_t max_age;
				switch (ages.size()) {
				case 1:
					min_age = boost::lexical_cast<age_t>(ages[0]);
					max_age = min_age;
					break;
				case 2:
					min_age = boost::lexical_cast<age_t>(ages[0]);
					if (!ages[1].empty()) {
						max_age = boost::lexical_cast<age_t>(ages[1]);
					} else {
						max_age = RateCalibrator::MAX_AGE;
					}
					break;
				default:
					throw DataException(boost::str(boost::format("RateCalibrator: cannot convert column name \"%s\" to an age group") % orig_str));
				}
				return age_group_type(min_age, max_age);*/
			}

			DataFrame<age_group_type, int, double> read_values(CSVFileReader& reader, flag_type flags) {
				const auto missing_year_handler = [&reader](const std::exception&, const std::string& elem) -> int {
					throw DataException(boost::str(boost::format("RateCalibrator: cannot convert \"%s\" to year number in file %s, column %d") % elem % reader.file_name() % YEAR_COLUMN_INDEX));
				};

				DataFrame<age_group_type, int> data(DataFrame<age_group_type, int>::from_csv_file(reader,
					age_group_from_string,
					CSVFileReader::default_converter<int>(missing_year_handler),
					(flags & USE_NANS_FOR_MISSING) != 0, 
					(flags & PAD_NAN_COLS) != 0));
				data.sort_index();
				
				const std::vector<age_group_type>& age_groups = data.columns();
				if ((flags & CHECK_AGE_GROUP_OVERLAP) && (age_groups.size() > 1)) {
					auto prev_it = age_groups.begin();
					for (auto next_it = prev_it + 1; next_it != age_groups.end(); ++next_it) {
						if (!prev_it->is_disjoint_with(*next_it)) {
							throw DataException(boost::str(boost::format("RateCalibrator: file %s: age group %s overlaps with %s") % reader.file_name() % *prev_it % *next_it));
						}
					}
				}
				return data;
			}

			DataFrame<age_group_type, int, double> read_values_and_interpolate(const std::vector<int>& years, CSVFileReader& reader, flag_type flags) {
				DataFrame<age_group_type, int, double> loaded_df(read_values(reader, flags));
				loaded_df.sort_index();
				const size_t ny = years.size();
				const size_t ng = loaded_df.nbr_cols();
				Eigen::MatrixXd values(ny, ng);
				for (size_t c = 0; c < ng; ++c) {
					const auto y = loaded_df.col_values_ix(c);
					for (size_t r = 0; r < ny; ++r) {
						values(r, c) = Interpolator::interpolate_y_linearly_or_extrapolate(loaded_df.index(), y, years[r]);
					}
				}
				return DataFrame<age_group_type, int, double>(values, loaded_df.columns(), years);
			}

            std::vector<int> merge_years(const std::vector<int>& years1, const std::vector<int>& years2) {
				return StlUtils::merge_sorted_vectors(years1, years2);
            }

            std::vector<int> realign_years(Eigen::MatrixXd& rates1, const std::vector<int>& years1, Eigen::MatrixXd& rates2, const std::vector<int>& years2) {
                const auto validator = [](const Eigen::MatrixXd& r, const std::vector<int>& y) {
                    if (y.empty()) {
                        throw std::domain_error("RatesCalibrator: empty years vector");
                    }
                    if (static_cast<size_t>(r.rows()) != y.size()) {
                        throw std::domain_error("RatesCalibrator: rates matrix / year vector size mismatch");
                    }
                };
                validator(rates1, years1);
                validator(rates2, years2);
                std::vector<int> years(merge_years(years1, years2));
                rates1 = realign_matrix_to_years(rates1, years1, years);
                rates2 = realign_matrix_to_years(rates2, years2, years);
                return years;
            }

            Eigen::MatrixXd realign_matrix_to_years(const Eigen::MatrixXd& rates, const std::vector<int>& years, const std::vector<int>& new_years) {
                const size_t r = years.size();
                const size_t nr = new_years.size();
                check_that(r <= nr);
				check_that(!years.empty());
				check_that(!new_years.empty());
				check_that(new_years.front() <= years.front());
				check_that(new_years.back() >= years.back());
				check_that(static_cast<size_t>(rates.rows()) == r);
                Eigen::MatrixXd new_rates(nr, rates.cols());
                size_t src_idx = r - 1;
                size_t dest_idx = nr - 1;
                for (; src_idx != static_cast<size_t>(-1); --src_idx) {
                    const int y = years[src_idx];
                    while (dest_idx != static_cast<size_t>(-1) && new_years[dest_idx] >= y) {
                        new_rates.row(dest_idx) = rates.row(src_idx);
                        --dest_idx;
                    }
                }
                for (; dest_idx != static_cast<size_t>(-1); --dest_idx) {
                    new_rates.row(dest_idx) = rates.row(0);
                }
                
                return new_rates;
            }

			void average_neighbouring_years(const Eigen::MatrixXd& values, const std::vector<int>& years, bool forward, Eigen::MatrixXd& new_values, std::vector<int>& new_years) {
				check_that(static_cast<size_t>(values.rows()) == years.size());
				check_that(!years.empty());
				const size_t nsize = years.size() - 1;
				new_years.resize(nsize);
				new_values.resize(nsize, values.cols());
				for (size_t i = nsize; i > 0; --i) {
					if (years[i] != years[i - 1] + 1) {
						throw DataException(boost::str(boost::format("RateCalibrator: years have a gap between %d and %d at index %d") % years[i] % years[i - 1] % i));
					}
					new_values.row(i - 1) = 0.5 * values.row(i) + 0.5 * values.row(i - 1);
					new_years[i - 1] = forward ? years[i] : years[i - 1];
				}
			}

			static int simulate_poisson_path(const double h, const double T, const double dt, RNG& rng) {
				assert(T >= 0);
				assert(dt >= 0);
				assert(h >= 0);
				int cnt = 0;
				double t = 0;
				while (t < T) {
					const double u = rng.next_uniform();
					const double ihr = HazardCurve::integrated_hazard_rate_from_jump_proba(u);
					const double tau = ihr / h;
					t += tau;
					if (t < T) {
						// stop after the event happens
						++cnt;
						t += dt;
					}
				}
				return cnt;
			}

			RunningStatistics<double> simulate_avg_number_jumps(const double h, const double T, const double dt, const unsigned int iters, RNG& rng) {
				RunningStatistics<double> rm;
				for (unsigned int i = 0; i < iters; ++i) {
					const int n = simulate_poisson_path(h, T, dt, rng);
					rm.add(static_cast<double>(n));
				}
				return rm;
			}

            RunningStatistics<double> simulate_avg_increment(const double h, const double T, const double dt, const unsigned int iters, const Distribution& jump_size_distr, RNG& rng) {
                check_that(jump_size_distr.infimum() >= 0.0);
                check_that(jump_size_distr.mean() > 0.0);
                // use independence property
                return simulate_avg_number_jumps(h, T, dt, iters, rng) * jump_size_distr.mean();
            }

			double hazard_rate_from_average_occurrences(const double T, const double N, const double dt, const unsigned int iters) {
				check_that(N >= 0, "RateCalibrator: N >= 0");
				check_that(dt >= 0, "RateCalibrator: dt >= 0");
				check_greater(T, dt, "RateCalibrator: T > dt");
				check_that(T > N * dt, "RateCalibrator: T > N*dt");
				check_that(std::isfinite(T), "RateCalibrator: T is finite");
				check_that(iters > 0, "RateCalibrator: iters > 0");
				if (N == 0) {
					return 0.0;
				}
				double h0 = N / T;
				if (dt == 0.0) {
					return h0;
				}
				assert(h0 > 0);
				const double cN = ceil(N);
				double h1;
				if (cN * dt == T) {
					h1 = N / (T - N * dt);
				} else {
					const double h1a = N / (T - N * dt);
					const double h1b = N / (T - cN * dt);
					h1 = std::max(h1a, h1b);
				}
				assert(h1 > h0);
				const double xtol = std::max(1e-8 * h0, 1e-8);
				const double ytol = 1e-8 * N;		
				static const size_t seed = 42;
				const auto fun = [N, T, dt, iters](double x) { 
					RNGImpl rng(seed);
					assert(dt > 0);
					// hope that MC errors cancel out between m1 and m0
					const double m1 = simulate_avg_number_jumps(x, T, dt, iters, rng).mean();
					rng = RNGImpl(seed);
					const double m0 = simulate_avg_number_jumps(x, T, 0, iters, rng).mean();
					//std::cerr << m1 << " " << m0 << " " << (x*T) << " " << N << "\n";
					return (m1 - m0) + x * T - N;					
				};
				// Find bounds for the root (necessary due to inaccuracies of Monte Carlo computations
				static const size_t max_tries = 10;
				size_t iter = 0;
				while (iter < max_tries && fun(h0) >= 0) {
					h0 *= 0.99;
					++iter;
				}
				iter = 0;
				while (iter < max_tries && fun(h1) <= 0) {
					h1 *= 1.5;
					//std::cerr << "new h1: " << h1 << std::endl;
					++iter;
				}
				double h;
				try {
					h = RootFinding::find_root(fun, h0, h1, xtol, ytol);
				} catch (std::runtime_error& e) {
					throw std::runtime_error(boost::str(boost::format("RateCalibrator: could not solve for h given N=%g, T=%g, dt=%g: %s") % N % T % dt % e.what()));
				}
				return std::max(h, N / T);
			}

            double hazard_rate_from_average_increment(double T, double N, double dt, const Distribution& jump_size_distr, const unsigned int iters) {
                check_that(jump_size_distr.infimum() >= 0.0);
				return hazard_rate_from_average_increment(T, N, dt, jump_size_distr.mean(), iters);
            }

			double hazard_rate_from_average_increment(double T, double N, double dt, const double mean_jump_size, const unsigned int iters) {
				check_that(mean_jump_size > 0.0, "Mean jump size must be positive");
				return hazard_rate_from_average_occurrences(T, N / mean_jump_size, dt, iters);
			}			

			std::vector<age_group_type> make_age_ranges(age_type step, age_type max_age, age_type start_age) {
				std::vector<age_group_type> age_ranges;
				age_ranges.reserve((max_age - start_age) / step + 1);
				age_type age = start_age;
				while (age < max_age) {
					age_ranges.push_back(age_group_type(age, age + step));
					age += step;
				}
				age_ranges.push_back(age_group_type(age, MAX_AGE + 1));
				return age_ranges;
			}
		}
	}
}
