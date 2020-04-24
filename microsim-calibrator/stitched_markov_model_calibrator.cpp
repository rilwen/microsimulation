/*
* (C) Averisera Ltd 2017
*/
#include "stitched_markov_model_calibrator.hpp"
#include "rate_calibrator.hpp"
#include "core/csv_file_reader.hpp"
#include "core/log.hpp"
#include "core/preconditions.hpp"
#include <boost/lexical_cast.hpp>
#include <algorithm>
#include <cassert>
#include <utility>

namespace averisera {
	namespace microsim {
		namespace StitchedMarkovModelCalibrator {
			static const Period PERIOD = Period::years(1);

			template <class S> void calibrate_annual_models(const age_type min_age, const year_type min_year_of_birth, const year_type min_year,
				const year_type max_year, const S dim, const Date::month_type month, const Date::day_type day,
				CSVFileReader& reader,
				std::vector<StitchedMarkovModelWithSchedule<S>>& models,
				std::vector<cohort_type>& cohorts
				) {
				typedef typename StitchedMarkovModel<S>::time_type time_type;
				check_that(max_year >= min_year);
				check_that(min_year_of_birth <= min_year);
				const year_type max_year_of_birth = max_year;
				assert(max_year_of_birth >= min_year_of_birth && "Max year of birth must not be less tham min year of birth");
				LOG_DEBUG() << "StitchedMarkovModelCalibrator: min_age=" << min_age << ", min_year_of_birth=" << min_year_of_birth << ", max_year_of_birth=" << max_year_of_birth << ", min_year=" << min_year << ", max_year=" << max_year << ", dim=" << dim << ", month=" << month << ", day=" << day;
				
				// load data
				typedef CalibrationTypes::age_group_type age_group_type;
				std::vector<Sex> data_sex;
				std::vector<std::string> data_ethnicity;
				std::vector<age_group_type> data_age_grp;
				std::vector<year_type> data_start_year;
				std::vector<Eigen::MatrixXd> data_pi;
				std::vector<Eigen::VectorXd> data_p0;

				reader.to_data();
				std::vector<std::string> data_row;
				while (reader.has_next_data_row()) {
					reader.read_data_row(data_row);
					data_sex.push_back(sex_from_string(data_row[0]));
					data_ethnicity.push_back(data_row[1]);
					data_age_grp.push_back(age_group_type::from_string_open_ended(data_row[2].c_str(), [](const std::string& str) { return boost::lexical_cast<age_type>(str); }, nullptr, &RateCalibrator::MAX_AGE));
					data_start_year.push_back(boost::lexical_cast<year_type>(data_row[3]));
					Eigen::MatrixXd pi(dim, dim);
					Eigen::VectorXd p0(dim);
					size_t i = 4;
					for (S r = 0; r < dim; ++r) {
						for (S c = 0; c < dim; ++c) {
							pi(r, c) = boost::lexical_cast<double>(data_row[i]);
							++i;
						}
					}
					for (S k = 0; k < dim; ++k) {
						p0[k] = boost::lexical_cast<double>(data_row[i]);
						++i;
					}
					check_equals<size_t, size_t, DataException>(i, data_row.size());
					data_pi.push_back(pi);
					data_p0.push_back(p0);					
				}
				const size_t nbr_read = data_sex.size();
				LOG_INFO() << "StitchedMarkovModelCalibrator: read " << nbr_read << " calibrated CSM models";
				if (!nbr_read) {
					return;
				}

				if (*std::min_element(data_start_year.begin(), data_start_year.end()) > min_year) {
					throw DataException("CSM models start too late");
				}

				// Assumes models loaded from reader are not overlapping
				typedef std::tuple<size_t, age_group_type> coverage_type; // (model index, age group covered)
				typedef std::vector<coverage_type> coverage_vector_type;
				std::map<cohort_type, coverage_vector_type> coverage_map;

				// get coverage info
				for (size_t i = 0; i < nbr_read; ++i) {
					const age_group_type age_grp = data_age_grp[i];
					const year_type start_year = data_start_year[i];
					const year_type min_yob = min_year_of_birth;
					const auto max_yob = static_cast<year_type>(max_year_of_birth - age_grp.begin());
					LOG_DEBUG() << "StitchedMarkovModelCalibrator: model=" << i << ", min_yob=" << min_yob << ", max_yob=" << max_yob << ", age_grp" << age_grp;
					if (min_yob > max_yob) {
						LOG_WARN() << "StitchedMarkovModelCalibrator: CSM model " << i << " is useless";
						continue;
					}
					const Sex sex = data_sex[i];
					const std::string& ethn = data_ethnicity[i];
					for (year_type yob = min_yob; yob <= max_yob; ++yob) {
						const cohort_type cohort(yob, ethn, sex);
						const age_type min_age = static_cast<age_type>(std::max(static_cast<int>(age_grp.begin()), start_year - yob));
						//LOG_DEBUG() << "StitchedMarkovModelCalibrator: min_age=" << min_age << " for yob=" << yob;
						if (min_age < age_grp.end()) {
							coverage_map[cohort].push_back(coverage_type(i, age_group_type(min_age, age_grp.end())));
						} else {
							LOG_WARN() << "StitchedMarkovModelCalibrator: model " << i << " is useless for year of birth " << yob;
						}
					}
				}

				const size_t cov_map_size = coverage_map.size();
				LOG_DEBUG() << "StitchedMarkovModelCalibrator: coverage map has " << cov_map_size << " elements";
				cohorts.reserve(cov_map_size);
				models.reserve(cov_map_size);
				// sort coverage info and create stitched models
				for (auto it = coverage_map.begin(); it != coverage_map.end(); ++it) {
					LOG_TRACE() << "StitchedMarkovModelCalibrator: cohort " << it->first;
					auto& cov_vec = it->second;
					if (cov_vec.empty()) {
						LOG_WARN() << "StitchedMarkovModelCalibrator: cohort " << it->first << " has no coverage";
						continue;
					}
					std::sort(cov_vec.begin(), cov_vec.end());
					LOG_DEBUG() << "StitchedMarkovModelCalibrator: coverage for cohort " << it->first << ": " << cov_vec;
					cohorts.push_back(it->first);
					const year_type yob = std::get<0>(it->first); // cohort year of birth
					const auto start_year = static_cast<year_type>(yob + std::get<1>(cov_vec.front()).begin());
					std::vector<Eigen::MatrixXd> intra_model_transition_matrices;
					std::vector<Eigen::VectorXd> initial_state_distributions;
					std::vector<time_type> model_lengths;
					const size_t n = cov_vec.size();
					assert(n > 0);
					intra_model_transition_matrices.reserve(n);
					initial_state_distributions.reserve(n);
					model_lengths.reserve(n - 1);			
					time_type prev_len = 0;
					for (const auto& ci : cov_vec) {
						const size_t csm_idx = std::get<0>(ci);
						const auto& age_grp = std::get<1>(ci);
						const auto yr = static_cast<year_type>(yob + age_grp.begin());
						const int shift = yr - data_start_year[csm_idx];
						Eigen::VectorXd p0(data_p0[csm_idx]);
						const Eigen::MatrixXd& pi = data_pi[csm_idx];
						for (int i = 0; i < shift; ++i) {
							p0 = pi * p0;
						}
						p0 /= p0.sum(); // normalise
						intra_model_transition_matrices.push_back(pi);
						initial_state_distributions.push_back(p0);
						if (yr > start_year) {
							model_lengths.push_back(prev_len);
						}
						prev_len = static_cast<time_type>(age_grp.end() - age_grp.begin() - 1);
					}
					models.push_back(StitchedMarkovModelWithSchedule<S>(StitchedMarkovModel<S>::ordinal(dim, intra_model_transition_matrices, initial_state_distributions, model_lengths), PERIOD, Date(start_year, month, day), Date(static_cast<year_type>(max_year + 1), month, day)));
				}
				LOG_DEBUG() << "StitchedMarkovModelCalibrator: generated " << models.size() << " models";
			}

			template void calibrate_annual_models<uint8_t>(const age_type min_age, const year_type min_year_of_birth, const year_type min_year, const year_type max_year, const uint8_t dim, const Date::month_type month, const Date::day_type day,
				CSVFileReader& reader, std::vector<StitchedMarkovModelWithSchedule<uint8_t>>& models, std::vector<cohort_type>& cohorts);

			template void calibrate_annual_models<uint16_t>(const age_type min_age, const year_type min_year_of_birth, const year_type min_year, const year_type max_year, const uint16_t dim, const Date::month_type month, const Date::day_type day,
				CSVFileReader& reader, std::vector<StitchedMarkovModelWithSchedule<uint16_t>>& models, std::vector<cohort_type>& cohorts);

			template void calibrate_annual_models<uint32_t>(const age_type min_age, const year_type min_year_of_birth, const year_type min_year, const year_type max_year, const uint32_t dim, const Date::month_type month, const Date::day_type day,
				CSVFileReader& reader, std::vector<StitchedMarkovModelWithSchedule<uint32_t>>& models, std::vector<cohort_type>& cohorts);
		}
	}
}
