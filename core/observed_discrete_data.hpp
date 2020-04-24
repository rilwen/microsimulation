/*
(C) Averisera Ltd 2014
*/
#ifndef __AVERISERA_OBSERVED_DISCRETE_DATA_H
#define __AVERISERA_OBSERVED_DISCRETE_DATA_H

#include <Eigen/Core>
#include <cstdint>
#include <iosfwd>
#include <vector>
#include "bootstrap.hpp"
#include "jagged_2d_array.hpp"

namespace averisera {

	// POD class for observed discrete data.
	struct ObservedDiscreteData {
		// Longitudinal data category index type
		typedef uint32_t lcidx_t; /**< Type for state index */
        typedef int64_t exp_lcidx_t; /**< Type of the element of an expanded trajectory (with -1 for missing data) */
        typedef size_t index_t; /**< Type to index trajectories and measure their size */

		/** Load trajectory data from a tab-delimited CSV file.

		First row in the file should contain years. Every other row should contain category indices (>=0) or -1 to denote missing value. E.g.

		2000,2001,2002
		-1,0,0
		0,0,1
		1,-1,1

		contains three trajectories: first starting in 2001 and equal (0, 0), second starting in 2000 and equal (0, 0, 1) and the third starting in 2000 and equal (1, 1) with a gap (missing value) in 2001.

		@param[in] fname Name of the file.
		@param[out] data Instance of ObservedDiscreteData structure to which trajectories will be loaded.

		@return Number of loaded trajectories.
		*/
        static index_t load_trajectories(const char* fname, ObservedDiscreteData& data);

        // Return pair (dim, T)
		static std::pair<lcidx_t, index_t> load_probabilities(const char* fname, ObservedDiscreteData& data, bool check_sum);

		// Create cross-sectional set with T time periods and dimension N
		// Sets numbers of surveys to 1 for each period and times to 0, 1, ..., T - 1
		ObservedDiscreteData(lcidx_t N = 0, index_t T = 0);

		// Construct longitudinal data set from vectors of vector.
		// times: Times of observations
		// trajs: Observed category values
		ObservedDiscreteData(const std::vector<std::vector<double>>& times, const std::vector<std::vector<lcidx_t>>& trajs);

		// Copy constructor
		ObservedDiscreteData(const ObservedDiscreteData& other);

		// Move constructor
		ObservedDiscreteData(ObservedDiscreteData&& other);

		ObservedDiscreteData& operator=(const ObservedDiscreteData& other);

		ObservedDiscreteData& operator=(ObservedDiscreteData&& other);

		void swap(ObservedDiscreteData& other);

		/// Observed probability distributions (in columns): probs(k, t) = P(X_t = k).
		Eigen::MatrixXd probs;

		/// Numbers of cross-sectional surveys for each t
		Eigen::VectorXd nbr_surveys;

		/// Times of cross-sectional observations.
		std::vector<double> times;

		/// Longitudinal trajectories.
		Jagged2DArray<lcidx_t> ltrajs;

		/// Observation times for longitudinal trajectories.
		Jagged2DArray<double> ltimes;

		/*
		Data operations
		*/

		// Pad observed cross-sectional data with empty observation periods to ensure that time increment is always 1 and that they cover the entire span of longitudinal data
		// data: Observed data with possible gaps
		// input_to_padded: At exit, maps input data indices to padded data indices
		static ObservedDiscreteData pad(const ObservedDiscreteData& data, std::vector<size_t>& input_to_padded);

		// Change time unit: times[i] --> times[i] / new_time_unit
		// new_time_unit: Positive value
		static void change_time_unit(ObservedDiscreteData& data, double new_time_unit);

		// Find smallest time increment in times vector
		// Return +Infinity for empty or size-1 vectors
		static double smallest_time_increment(const std::vector<double>& times);

		// Change the time unit to smallest time increment to ensure it is discrete
		// Return new time unit.
		static double discretize_times(ObservedDiscreteData& data);

		// Find the earliest observation time or return +Infinity if empty
		static double first_time(const ObservedDiscreteData& data);

		// Find the last observation time or return -Infinity if empty
		static double last_time(const ObservedDiscreteData& data);

		// Get the dimension of the observed process
		static lcidx_t dim(const ObservedDiscreteData& data);

		// Get the time size of the data
		static index_t T(const ObservedDiscreteData& data) {
			return static_cast<unsigned int>(last_time(data) - first_time(data) + 1);
		}

		// Check if the data contain longitudinal trajectories
		static bool has_trajectories(const ObservedDiscreteData& data) {
			return data.ltrajs.nbr_elements() > 0;
		}

		// Resample data for bootstrapping
		template <class R> static ObservedDiscreteData resample(Bootstrap<R>& bootstrap, const ObservedDiscreteData& data);		

		// Reduce a fraction of trajectories in mixed cross-sectional+longitudinal data to cross-sectional (loses information!)
		static ObservedDiscreteData to_cross_sectional(const ObservedDiscreteData& data, double fract);

		// Save cross-sectional data to stream
		static void cross_sectional_to_stream(std::ostream& stream, const ObservedDiscreteData& data);

		static bool empty(const ObservedDiscreteData& data) {
			return data.probs.size() == 0 && data.ltrajs.nbr_elements() == 0;
		}

		template <class V> static index_t count_specified_states(const V& expanded_trajectory, const index_t t, const index_t memory) {
			const index_t overlap_len = std::min(memory, t) + 1;
			index_t nbr_specified = 0;
			for (index_t q = 0; q < overlap_len; ++q) {
				if (expanded_trajectory[t - q] >= 0) {
					++nbr_specified;
				}
			}
			return nbr_specified;
		}

		/** Convert trajectory from sparse to dense format, inserting -1 in elements where we don't have data. 
		First element of expanded_traj corresponds to t == min_time.
		*/
		template <class V> static void expand_trajectory(const Jagged2DArrayRowRef<lcidx_t>& traj, const Jagged2DArrayRowRef<double>& times, const double min_time, V& expanded_traj) {
			std::fill(expanded_traj.begin(), expanded_traj.end(), -1);  // -1 means we do not know the state
			const size_t traj_T = traj.size();
			for (size_t t = 0; t < traj_T; ++t) {
				expanded_traj[static_cast<size_t>(times[t] - min_time)] = traj[t];
			}
		}

		template <class V> static bool state_index_compatible_with_data(const V& expanded_trajectory, const std::vector<size_t>& indices, index_t memory, index_t t) {
			const index_t overlap_len = std::min(memory, t) + 1;
			for (index_t q = 0; q < overlap_len; ++q) {
				const exp_lcidx_t d_i = expanded_trajectory[t - q];
				if (d_i >= 0 && indices[q] != static_cast<size_t>(d_i)) {
					return false;
				}
			}
			return true;
		}

		/// Validate that the data are correct.
		/// @throw DataException If there are problems with data.
		void validate() const;
	};	

	template <class R> ObservedDiscreteData ObservedDiscreteData::resample(Bootstrap<R>& bootstrap, const ObservedDiscreteData& data) {
		ObservedDiscreteData resampled(data);
		const size_t T = data.probs.cols();
		check_equals(T, data.nbr_surveys.size());
		for (size_t t = 0; t < T; ++t) {
			const auto in = data.probs.col(t);
			auto out = resampled.probs.col(t);
			bootstrap.resample_distribution(in, static_cast<size_t>(data.nbr_surveys[t]), out);
		}
		const size_t nt = data.ltrajs.size();
		std::vector<size_t> idx_out(nt);
		{
			std::vector<size_t> resampled_traj_sizes(nt);
			{
				std::vector<size_t> idx_in(nt);
				std::iota(idx_in.begin(), idx_in.end(), 0);
				bootstrap.resample_with_replacement(idx_in, idx_out);
			}
			for (size_t i = 0; i < nt; ++i) {
				resampled_traj_sizes[i] = data.ltrajs.row_size(idx_out[i]);
			}
			resampled.ltrajs = Jagged2DArray<lcidx_t>::from_iters(resampled_traj_sizes.begin(), resampled_traj_sizes.end());
			resampled.ltimes = Jagged2DArray<double>::from_iters(resampled_traj_sizes.begin(), resampled_traj_sizes.end());
		}
		for (size_t i = 0; i < nt; ++i) {
			const size_t idx = idx_out[i];
			const auto ltrajs_row = data.ltrajs[idx];
			std::copy(ltrajs_row.begin(), ltrajs_row.end(), resampled.ltrajs[i].begin());
			const auto ltimes_row = data.ltimes[idx];
			std::copy(ltimes_row.begin(), ltimes_row.end(), resampled.ltimes[i].begin());
		}
		return resampled;
	}

	inline void swap(ObservedDiscreteData& l, ObservedDiscreteData& r) {
		l.swap(r);
	}
}

#endif
