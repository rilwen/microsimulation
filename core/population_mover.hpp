#pragma once
#include "preconditions.hpp"
#include "rng.hpp"
#include "segment_search.hpp"
#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
#include <Eigen/Core>
#include <gtest/gtest_prod.h>

namespace averisera {
	/** Implements population transfers between categories in a population */
	class PopulationMover {
	public:
		/*
		@param pi Transition matrix with conditional distributions in columns
		@param ranges Contains boundaries of ranges (size N+1 for N ranges) of X variable
		@param smooth If true, smoothen out the kinks in the generated distribution
		@throw std::domain_error ranges.size() < 2, pi.rows() != pi.cols(), pi.rows() + 1 != ranges.size()
		*/
		PopulationMover(const Eigen::MatrixXd& pi, const std::vector<double>& ranges, bool smooth = true);

		/** Represents a member of an abstract population */
		struct Member {
			size_t member_index; /**< Index of the member in the population */
			size_t distribution_index; /**< Which distribution to apply to select the next value */
			double value; /**< Current or new value */
			size_t range_index; /**< New range index */

			static double value_getter(const Member& m) {
				return m.value;
			}

			static size_t distr_getter(const Member& m) {
				return m.distribution_index;
			}

			static bool comparator(const Member& l, const Member& r) {
				if (l.distribution_index != r.distribution_index) {
					return l.distribution_index < r.distribution_index;
				} else {
					return l.value < r.value;
				}
			}
			
			/** Compares only values */
			static bool value_comparator(const Member& l, const Member& r) {
				return l.value < r.value;
			}
		};

		

		/**
		Move members between ranges of X variable, trying to minimise the magnitude of X value change for every moved member. For every i and j, preserves the X value ordering of members moved from range i to j.
		@param population Population composed of pairs (T, x) where T is the population object (or pointer to it) and x is its old value of X variable.		
		@throw DataException If some population member falls outside the ranges supported
		*/
		template <class T> void move_between_ranges(std::vector<std::pair<T, double>>& population, RNG& rng) const {
			check_equals(pi_.cols(), pi_.rows(), "Transition matrix must be square");
			std::vector<Member> members(get_member_info_no_memory(population));
			move_between_ranges(members, rng);
			for (const Member& m : members) {
				population[m.member_index].second = m.value;
			}
		}

		/** Abstract algorithm. Applies the "triangles" moving algorithm and then smooths the resulting distribution preserving the new ordering.

		Requirements of members m in population:
		1. for members m with given m.distribution_idx every m.value must fall in the same range (as given by ranges_)

		On return, elements of population vector have new value and range_index set but their other properties are not changed. The are reordered though.

		@throw std::domain_error If population requirements not met.
		*/
		void move_between_ranges(std::vector<Member>& population, RNG& rng) const;

		const std::vector<double>& ranges() const {
			return ranges_;
		}

		friend class PopulationMoverTest;
		FRIEND_TEST(PopulationMoverTest, draw_moved_indices);
		FRIEND_TEST(PopulationMoverTest, get_range_indices_distrs);
		FRIEND_TEST(PopulationMoverTest, move_and_draw_new_values);
		FRIEND_TEST(PopulationMoverTest, approximate_linear_pdf);
	private:		
		// Draw a number from a distribution with PDF increasing linearly from rho(a) == 0 to rho(b) == 0.5 / (b - a)
		static double draw_from_linear_pdf(double a, double b, RNG& rng);

		//static double approximate_linear_pdf(double x, double dx);

		// Make sure that every range gets at least a small probability
		std::vector<double> calc_new_probabilities(const std::vector<size_t>& range_counts, double popsize) const;

		// implementation
		template <class T> std::vector<size_t> get_range_indices(const std::vector<std::pair<T, double>>& sorted_population) const {
			return get_range_indices(ranges_, sorted_population, [](const std::pair<T, double>& p) {return p.second; });
		}

		// Return a vector of indices ri such that all population members with indices < ri[k] have values less than ranges[k], and those with indices >= ri[k] have values >= ranges[k]
		template <class S, class T, class G> static std::vector<size_t> get_range_indices(const std::vector<S>& ranges, const std::vector<T>& sorted_vector, G value_getter) {
			std::vector<size_t> range_indices;
			assert(!sorted_vector.empty());
			assert(!ranges.empty());
			range_indices.reserve(ranges.size());
			const size_t popsize = sorted_vector.size();
			assert(ranges.front() <= value_getter(sorted_vector.front()));
			assert(ranges.back() >= value_getter(sorted_vector.back()));
			const auto it_end = ranges.end() - 1;
			range_indices.push_back(0);
			for (auto it = ranges.begin() + 1; it != it_end; ++it) {
				// Find smallest i such that population[i] has value >= x
				const size_t i = SegmentSearch::binary_search_right_inclusive(sorted_vector, popsize, *it, [value_getter](const std::vector<T>& v, size_t idx) { return value_getter(v[idx]); }) + 1;
				assert(i <= popsize);
				assert(i >= range_indices.back());
				range_indices.push_back(i);
			}
			range_indices.push_back(popsize);
			//std::cout << range_indices.front() << ", " << range_indices[1] << " to " << range_indices.back() << std::endl;
			return range_indices;
		}

		// Select indices which should be moved to another column
		static void draw_moved_indices(Eigen::Ref<const Eigen::VectorXd> distr, const size_t i0, const size_t from_size, const size_t range_from, std::vector<std::vector<size_t>>& transferred_indices, RNG& rng, std::vector<double>& local_probs);

		// Move selected members and draw new values for them
		void move_and_draw_new_values(const std::vector<size_t>& ti, std::vector<double>& new_x_values, const size_t from, const size_t to, const std::vector<Member>& population, std::vector<Member>& new_population, RNG& rng) const;

		template <class T> std::vector<Member> get_member_info_no_memory(const std::vector<std::pair<T, double>>& population) const {
			check_equals(range_cnt_, distr_cnt_);
			const size_t popsize = population.size();
			std::vector<Member> info;
			if (!population.empty()) {
				info.reserve(popsize);
				size_t idx = 0;
				for (const auto& p : population) {
					info.push_back({ idx, 0, p.second, 0 });
					++idx;
				}
				std::sort(info.begin(), info.end(), Member::comparator); // use the fact that all members have distribution_index == 0
				const std::vector<size_t> range_indices(get_range_indices(ranges_, info, Member::value_getter));
				assert(range_indices.size() == range_cnt_ + 1);
				for (size_t i = 0; i < range_cnt_; ++i) {
					const size_t j_end = range_indices[i + 1];
					for (size_t j = range_indices[i]; j < j_end; ++j) {
						info[j].distribution_index = i;
                        info[j].range_index = i;
					}
				}
			}
			return info;
		}

		Eigen::MatrixXd pi_;
		std::vector<double> ranges_;
		std::vector<size_t> distrs_;
		size_t range_cnt_; /**< Number of ranges */
		size_t distr_cnt_; /**< Number of conditional distributions */
		bool smooth_;
	};
}
