// (C) Averisera Ltd 2014-2020
//#pragma once	
//#include "population_mover.hpp"
//#include <vector>
//#include <Eigen/Core>
//
//namespace averisera {
//	class RNG;
//
//	/** Multi-dimensional version of PopulationMover */
//	class PopulationMoverMultidim {
//	public:
//		/**
//		@param pi Transition matrix with conditional distributions in columns
//		@param ranges Vector of vectors of boundaries of ranges (size N+1 for N ranges) of X[i] variable
//		@param smooth If true, smoothen out the kinks in the generated marginal distributions
//		*/
//		PopulationMoverMultidim(const Eigen::MatrixXd& pi, const std::vector<std::vector<double>>& ranges, bool smooth = true);
//
//		/** Represents a member of an abstract population */
//		struct Member {
//			size_t member_index; /**< Index of the member in the population */
//			size_t distribution_index; /**< Which distribution to apply to select the next value */
//			std::vector<double> values; /**< Vector of current or new multidimensional values */
//			std::vector<size_t> range_indices; /**< Vector of new value range indices */
//		};
//
//		/** @see PopulationMover::move_between_ranges */
//		void move_between_ranges(std::vector<Member>& population, RNG& rng) const;
//	private:
//		size_t ndims_;
//		std::vector<unsigned int> dims_;
//		std::vector<PopulationMover> movers_;		
//
//		static void update_component(const Member& member, PopulationMover::Member& member_1d, size_t i, unsigned int prev_dim);
//
//		void initialize_populations(std::vector<Member>& population, std::vector<PopulationMover::Member>& pop1d) const;
//	};
//}
