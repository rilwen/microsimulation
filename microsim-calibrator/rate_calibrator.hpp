// (C) Averisera Ltd 2014-2020
#pragma once
#include "calibration_types.hpp"
#include "core/data_exception.hpp"
#include "core/data_frame.hpp"
#include "core/dates_fwd.hpp"
#include "core/generic_distribution_integral.hpp"
#include "core/preconditions.hpp"
#include "core/numerical_range.hpp"
#include "core/stl_utils.hpp"
#include <Eigen/Core>
#include <cstdint>
#include <utility>
#include <vector>

namespace averisera {
	class CSVFileReader;
    class Distribution;		
	class RNG;
	template <class T> class RunningStatistics;	
	class SamplingDistribution;

	namespace microsim {
		/** Methods used to calibrate rates of various phenomena, e.g. death */
		namespace RateCalibrator {
			using namespace CalibrationTypes;
			typedef Date::year_type year_t;
			const age_type MAX_AGE = 100000; /**< Maximum age supported by the calibrator: oldest living tree colony on Earth has about 80,000 years. */
			
			typedef uint32_t flag_type;
			const flag_type CHECK_AGE_GROUP_OVERLAP = 1 << 0;
			const flag_type AGGREGATE_TO_COARSER_AGE_GROUP = 1 << 1;
			const flag_type USE_NANS_FOR_MISSING = 1 << 2; /**< If set, replace missing values with NaNs, else throw an error */
			const flag_type PAD_NAN_COLS = 1 << 3; /** Pad NaN values in columns */

			/** Calculate rates of occurrence. Checks data for correctness.
			@throw std::domain_error If matrix dimensions do not match
			@throw DataException If data are incorrect for rate calculation */
			Eigen::MatrixXd calculate_rates(const Eigen::MatrixXd& occurrence_counts, const Eigen::MatrixXd& group_sizes);

			/** Calculate rates of occurrence. Checks data for correctness.
			@throw std::domain_error If matrix dimensions do not match			
			@throw DataException If data are incorrect for rate calculation or indices/columns do not match. */
			template <class C, class I> DataFrame<C, I, double> calculate_rates(const DataFrame<C, I, double>& occurrence_counts, const DataFrame<C, I, double>& group_sizes) {
				if (occurrence_counts.index() != group_sizes.index()) {
					throw DataException("RateCalibrator::calculate_rates: indices do not match");
				}
				if (occurrence_counts.columns() != group_sizes.columns()) {
					throw DataException("RateCalibrator::calculate_rates: columns do not match");
				}
				return DataFrame<C, I, double>(calculate_rates(occurrence_counts.values(), group_sizes.values()), occurrence_counts.columns(), occurrence_counts.index());
			}

			/** Calculate rates of occurrence, aggregating age groups if needed to make the matrix dimensions match.
			*/
			DataFrame<age_group_type, int, double> aggregate_and_calculate_rates(const DataFrame<age_group_type, int, double>& occurrences, const DataFrame<age_group_type, int, double>& group_sizes);

			DataFrame<age_group_type, int, double> read_and_calculate_rates(CSVFileReader& occurrence_reader, CSVFileReader& group_size_reader, flag_type flags);

			const std::string& YEAR_COLUMN_NAME();
			const extern size_t YEAR_COLUMN_INDEX;

			/** Convert string like "0-4" to numerical range [0, 5) */
			age_group_type age_group_from_string(const std::string& orig_str);

			/**
			First column of the file is expected to contain the year numbers.
			The subsequent columns should contain rates for age groups. Their names should be of the form "N-M", where N is the minimum and M the maximum age in an age group, or "N-" meaning "for any age N or higher", in which case we will assume MAX_AGE as the upper limit, or "N" in which case we will assume that minimum and maximum age are the same.
			*/
			DataFrame<age_group_type, int, double> read_values(CSVFileReader& reader, flag_type flags);
			
			/** Like read_values, but interpolates linearly / extrapolates flat values in the years (index) dimension.
			Sorts the index in ascending order. */
			DataFrame<age_group_type, int, double> read_values_and_interpolate(const std::vector<int>& years, CSVFileReader& reader, flag_type flags);

            /** Join years serii into a common sorted series of unique years. 
              @param years1 Sorted vector of unique years
              @param years2 Sorted vector of unique years
            */
            std::vector<int> merge_years(const std::vector<int>& years1, const std::vector<int>& years2);

            /** Realign (years1, rates1) and (years2, rates2) into a common years superset. Repeat matrix rows forward (and first one optionally backwards) to cover a wider range of years.
              @param years1 Sorted vector of unique years
              @param years2 Sorted vector of unique years
              @throw std::domain_error If rates1.rows() != years1.size(), years1.empty(), or similar for (rates2, years2)
              @return Vector of all years
            */
            std::vector<int> realign_years(Eigen::MatrixXd& rates1, const std::vector<int>& years1, Eigen::MatrixXd& rates2, const std::vector<int>& years2);

            /** Repeat matrix rows forward (and first one optionally backwards) to cover a wider range of years.
			@throw std::domain_error If dimensions do not match or years.empty()
             */
            Eigen::MatrixXd realign_matrix_to_years(const Eigen::MatrixXd& rates, const std::vector<int>& years, const std::vector<int>& new_years);

			/** Average neighbouring years for values measured over subsequent years (rows).
			@param forward If true, the average of Y and Y+1 is assigned to Y+1, else to Y.
			@throw std::domain_error If values.rows() != years.size()
			@throw DataException If years have gaps
			*/
			void average_neighbouring_years(const Eigen::MatrixXd& values, const std::vector<int>& years, bool forward, Eigen::MatrixXd& new_values, std::vector<int>& new_years);

            // TODO: generalize this into a stochastic process model and move to core. 

            /**
              Simulate Poisson process with hazard rate h and post/pre-jump halt time dt.
             */
			RunningStatistics<double> simulate_avg_number_jumps(const double h, const double T, const double dt, const unsigned int iters, RNG& rng);

            /**
              Simulate "Poisson" process with hazard rate h and post/pre-jump halt time dt. The jump size is a non-negative real number drawn from jump_size_distr. Mean value of this distribution must be positive.
              @throw std::domain_error If distribution.infimum() < 0 or distribution.mean() <= 0.
             */
			RunningStatistics<double> simulate_avg_increment(const double h, const double T, const double dt, const unsigned int iters, const Distribution& jump_size_distr, RNG& rng);
			
			/** Let X be an event occurring over a time period dt. We assume it is triggered by Poisson process with hazard rate h. The process stops running for time dt after X is triggered and resumes afterwards. Calculate h given the average number N of times X occurs during time T.

			@param T > (N - 1) * dt and T < infty
			@param N >= 0
			@param dt >= 0
			@throw std::out_of_range If above inequalities are violated.
			*/
			double hazard_rate_from_average_occurrences(double T, double N, double dt, unsigned int iters = 10000);

            /** The same but with random jump size. */
            double hazard_rate_from_average_increment(double T, double N, double dt, const Distribution& jump_size_distr, unsigned intiters = 10000);
			double hazard_rate_from_average_increment(double T, double N, double dt, const double mean_jump_size, unsigned intiters = 10000);

			/** @tparam T integral type */
			template <class JumpSize> double hazard_rate_from_average_increment(double T, double N, double dt, const GenericDistribution<JumpSize>& jump_size_distr) {
				return hazard_rate_from_average_increment(T, N, dt, GenericDistributionIntegral<JumpSize>::mean(jump_size_distr));
			}

			/** Given values for old age groups [x, y] and [y, z], v1 and v2 respectively, calculate the value for the new age group [x, z] as v1 + v2. 			
			@throw DataException If new age groups cannot be formed from old age groups.
			*/
			template <class I> DataFrame<age_group_type, I> aggregate_age_groups(const DataFrame<age_group_type, I>& data, const std::vector<age_group_type>& new_groups) {
				typedef DataFrame<age_group_type, I> df_type;
				if (new_groups.empty()) {
					return df_type(Eigen::MatrixXd(data.nbr_rows(), 0), new_groups, data.index());
				}
				const size_t nog = data.nbr_cols();
				const size_t nng = new_groups.size();
				check_that<DataException>(nng <= nog, "RateCalibrator: Too many new age groups");
				if (nng > 1) {
					auto prev_it = new_groups.begin();
					for (auto next_it = prev_it + 1; next_it != new_groups.end(); ++next_it) {
						if (!prev_it->is_disjoint_with(*next_it)) {
							throw DataException(boost::str(boost::format("RateCalibrator: age group %s overlaps with %s") % *prev_it % *next_it));
						}
					}
				}
				const auto& values = data.values();
				const auto& old_groups = data.columns();
				Eigen::MatrixXd new_values(values.rows(), new_groups.size());
				new_values.setZero();
				size_t dest_col_idx = 0;
				for (size_t i = 0; i < nog; ++i) {
					const age_group_type& grp_i = old_groups[i];
					//const age_t start = grp_i.begin();
					//const age_t end = grp_i.end();
					const auto does_not_fit = [&grp_i, &new_groups](size_t dci) {return !new_groups[dci].contains(grp_i); };
						//start < new_groups[dci].begin() || end > new_groups[dci].end(); };
					while (dest_col_idx < nng && does_not_fit(dest_col_idx)) {
						++dest_col_idx;
					}
					if (dest_col_idx == nng) {
						throw DataException(boost::str(boost::format("RateCalibrator: old age group %s does not fit") % grp_i));
					}
					if (does_not_fit(dest_col_idx)) {
						throw DataException(boost::str(boost::format("RateCalibrator: old age group %s does not fit in new group %s") % grp_i % new_groups[dest_col_idx]));
					}
					new_values.col(dest_col_idx).noalias() += values.col(i);
				}
				return df_type(new_values, new_groups, data.index());
			}

			/** Make sequence of age ranges.

			Let remainder := (max_age - start_age) % step.
			If remainder == 0, returns [start_age, start_age + step), [start_age + step, start_age + 2 * step), ..., [max_age, MAX_AGE + 1). Otherwise, returns [start_age, start_age + step), [start_age + step, start_age + 2 * step), ..., [max_age + remainder - step, max_age + remainder), [max_age + remainder, MAX_AGE + 1).
			*/
			std::vector<age_group_type> make_age_ranges(age_type step, age_type max_age, age_type start_age = 0);
		} // RateCalibrator
	}
}
