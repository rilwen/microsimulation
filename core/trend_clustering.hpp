#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "preconditions.hpp"
#include <Eigen/Core>
#include <gtest/gtest_prod.h>
#include <vector>

namespace averisera {
	/** Finds trends in clusters */
	class TrendClustering {
	public:
		/**
		Given series x, return a matrix with D^p x , ..., D^q x in subsequent rows.

		If forward == true, the discrete derivative operator D is defined as
		\[
		(Dx)_i :=
		\begin{cases}
		x_{i+1} - x_i		& i < N - 1
		x_{N-1} - x_{N-2}	& i = N - 1
		\end{cases}
		\]
		If forward == false, D is defined as
		\[
		(Dx)_i :=
		\begin{cases}
		x_i - x_{i-1}		& i > 0
		x_1 - x_0			& i = 0
		\end{cases}
		\]

		@param series Vector with length n
		@param p
		@param q
		@param forward Use forward derivatives
		@return (q - p + 1) x n matrix		
		@throws std::domain_error if p > q
		*/
		static Eigen::MatrixXd calculate_derivatives(const std::vector<double>& series, unsigned int p, unsigned int q, bool forward);

		/**
		Given serii x and y, return a matrix with D^p y , ..., D^q y in subsequent rows.

		If forward == true, the discrete derivative operator D is defined as
		\[
		(Dy)_i :=
		\begin{cases}
		(y_{i+1} - y_i) / (x_{i+1} - x_i)			& i < N - 1
		(y_{N-1} - y_{N-2}) / (x_{N-1} - x_{N-2})	& i = N - 1
		\end{cases}
		\]
		If forward == false, D is defined as
		\[
		(Dy)_i :=
		\begin{cases}
		(y_i - y_{i-1}) / (x_i - x_{i-1})			& i > 0
		(y_1 - y_0) / (x_1 - x_0)					& i = 0
		\end{cases}
		\]

		@param xy Matrix with X data in 1st row and Y data in 2nd row
		@param p
		@param q
		@param forward Use forward derivatives
		@return (q - p + 1) x n matrix
		@throws std::domain_error if p > q
		*/
		static Eigen::MatrixXd calculate_derivatives(const Eigen::MatrixXd& xy, unsigned int p, unsigned int q, bool forward);

		/**
		Check if clusters form trends and find boundaries between them.
		@param[in] assignments Cluster assignments for each series point
		@param[in] k Number of clusters, > 0
		@param[out] trends Maps clusters to trends (j-th cluster is mapped to trends[j]-th trend)
		@param[out] boundaries Resized to k - 1. Points with indices i in [boundaries[l - 1], boundaries[l]) are classified as trend l; boundaries[-1] == 0
		@throw std::domain_error If k == 0
		@return Vector with numbers of misclassified points per cluster
		*/
		static std::vector<size_t> map_clusters_in_trends(const std::vector<unsigned int>& assignments, unsigned int k, std::vector<unsigned int>& trends, std::vector<size_t> & boundaries);

		/** Reassign points from clusters to trends. 
		@param[in] trends Maps clusters to trends (j-th cluster is mapped to trends[j]-th trend)
		@param[in,out] assignments In: cluster assignments for each series point. Out: trend assignment for each series point.
		*/
		static void reassign_to_trends(const std::vector<unsigned int>& trends, std::vector<unsigned int>& assignments) {
			for (unsigned int& j : assignments) {
				j = trends[j];
			}
		}

		/** Generate a sample for k-means clustering 
		@throws std::domain_error If series.size() < 2
		*/
		static Eigen::MatrixXd make_sample_for_kmeans_clustering(const std::vector<double>& series);
	private:
		/** Differentiate input into output using one-sided backward-looking derivatives. */
		template <class V1, class V2> static void differentiate_once_backward(const V1& input, V2& output);

		/** Differentiate y(x) into output using one-sided backward-looking derivatives. */
		template <class V1, class V2, class V3> static void differentiate_once_backward(const V1& x, const V2& y, V3& output);

		/** Differentiate input into output using one-sided backward-looking derivatives. */
		template <class V1, class V2> static void differentiate_once_forward(const V1& input, V2& output);

		/** Differentiate y(x) into output using one-sided backward-looking derivatives. */
		template <class V1, class V2, class V3> static void differentiate_once_forward(const V1& x, const V2& y, V3& output);

		static std::vector<size_t> calc_cluster_sizes(const std::vector<unsigned int>& assignments, const unsigned int k);

		/** Calculates lower median*/
		static std::vector<size_t> calc_median_indices(const std::vector<unsigned int>& assignments, const std::vector<size_t>& cluster_sizes, const unsigned int k);

		static std::vector<double> calc_mean_indices(const std::vector<unsigned int>& assignments, const unsigned int k);
		FRIEND_TEST(TrendClusteringTest, calc_mean_indices);

		static std::vector<unsigned int> sort_clusters_by_median_index(const std::vector<size_t>& medians, const unsigned int k);
		
		static std::vector<unsigned int> calc_cluster_ranks(const std::vector<unsigned int>& cluster_indices_sorted, const unsigned int k);
		
		static void initialise_boundaries(const std::vector<size_t>& cluster_sizes, const std::vector<unsigned int>& cluster_indices_sorted, const unsigned int k, std::vector<size_t> & boundaries);

		static std::vector<size_t> calc_errors_per_cluster(const std::vector<unsigned int>& assignments, const std::vector<unsigned int>& cluster_indices_sorted, const std::vector<size_t> & boundaries, const unsigned int k);
		
		FRIEND_TEST(TrendClusteringTest, test_internal);
	};
}
