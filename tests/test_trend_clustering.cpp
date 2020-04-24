/*
(C) Averisera Ltd 2017
*/
#include <numeric>
#include <gtest/gtest.h>
#include "core/kmeans.hpp"
#include "core/kmeans_init_strats.hpp"
#include "core/rng_impl.hpp"
#include "core/stl_utils.hpp"
#include "core/trend_clustering.hpp"

namespace averisera {
	TEST(TrendClusteringTest, Linear) {
		const size_t n = 20;
		const size_t n0 = 15;
		std::vector<double> series(n, 0.0);
		std::iota(series.begin() + n0, series.end(), 1.0);
		//const Eigen::MatrixXd sample(TrendClustering::differentiate(series, 0, 2));
		const Eigen::MatrixXd sample(TrendClustering::make_sample_for_kmeans_clustering(series));
		const KMeans kmeans(KMeansInitStrategies::make_kmeanspp());
		RNGImpl rng(42);
		const unsigned int k = 2;
		const std::vector<unsigned int> assignments2(kmeans.clusterise(sample, k, rng));
		std::vector<unsigned int> trends2;
		std::vector<size_t> boundaries2;
		const auto nerr2 = TrendClustering::map_clusters_in_trends(assignments2, k, trends2, boundaries2);
		ASSERT_EQ(0u, std::accumulate(nerr2.begin(), nerr2.end(), size_t(0)));
		ASSERT_EQ(1u, boundaries2.size());
		ASSERT_EQ(2u, trends2.size());
		ASSERT_TRUE(std::abs(static_cast<int>(n0 - boundaries2[0])) <= 1) << boundaries2[0];


	}

	/*TEST(TrendClusteringTest, Linear3) {
		const size_t n = 20;
		const int n0 = 15;
		std::vector<double> series(n, 0.0);
		std::iota(series.begin() + n0, series.end(), 1.0);
		const Eigen::MatrixXd sample(TrendClustering::differentiate(series, 0, 2));
		const KMeans kmeans(KMeansInitStrategies::make_kmeanspp());
		RNGImpl rng(42);
		const unsigned int k = 3;
		const std::vector<unsigned int> assignments3(kmeans.clusterise(sample, k, rng));
		std::vector<unsigned int> trends3;
		std::vector<size_t> boundaries3;
		const auto nerr3 = TrendClustering::map_clusters_in_trends(assignments3, k, trends3, boundaries3);
		ASSERT_EQ(2u, boundaries3.size());
		ASSERT_EQ(3u, nerr3.size());
		for (int i = 0; i < 2; ++i) {
			ASSERT_LE(std::abs(static_cast<int>(boundaries3[i]) - n0), 3);
		}
		ASSERT_EQ(0u, std::accumulate(nerr3.begin(), nerr3.end(), size_t(0)));
	}*/

	TEST(TrendClusteringTest, Quadratic) {
		const size_t n = 20;
		const size_t n0 = 15;
		std::vector<double> series(n, 0.0);
		for (size_t i = n0; i < n; ++i) {
			series[i] = std::pow(i - n0, 2);
		}
		//const Eigen::MatrixXd sample(TrendClustering::differentiate(series, 0, 2));
		const Eigen::MatrixXd sample(TrendClustering::make_sample_for_kmeans_clustering(series));
		const KMeans kmeans(KMeansInitStrategies::make_kmeanspp());
		RNGImpl rng(42);
		const std::vector<unsigned int> assignments2(kmeans.clusterise(sample, 2, rng));
		std::vector<unsigned int> trends2;
		std::vector<size_t> boundaries2;
		const auto nerr2 = TrendClustering::map_clusters_in_trends(assignments2, 2, trends2, boundaries2);
		ASSERT_EQ(0u, std::accumulate(nerr2.begin(), nerr2.end(), size_t(0)));
		ASSERT_EQ(1u, boundaries2.size());
		ASSERT_EQ(2u, trends2.size());
		ASSERT_TRUE(std::abs(static_cast<int>(n0 - boundaries2[0])) <= 3) << boundaries2[0];
	}

	TEST(TrendClusteringTest, Pathological) {
		const std::vector<double> series(5, 1.0);
		const Eigen::MatrixXd sample(TrendClustering::make_sample_for_kmeans_clustering(series));
		//const Eigen::MatrixXd sample(TrendClustering::differentiate(series, 0, 2));
		const KMeans kmeans(KMeansInitStrategies::make_kmeanspp());
		RNGImpl rng(42);
		const unsigned int k = 2;
		const std::vector<unsigned int> assignments(kmeans.clusterise(sample, k, rng));
		std::vector<unsigned int> trends;
		std::vector<size_t> boundaries;
		const auto nerr2 = TrendClustering::map_clusters_in_trends(assignments, k, trends, boundaries);
	}

    // OFF because we don't use trend clustering for BMI anymore
	// TEST(TrendClusteringTest, BMI) {
	// 	const std::vector<double> series({ 0.152911194, 0.160941828, 0.169708412, 0.181377826, 0.188480996, 0.198504028, 0.204371912, 0.214356226, 0.228124295, 0.206971942, 0.237272081, 0.242289935, 0.253047294, 0.255311857, 0.25498699, 0.260244255, 0.252142301, 0.27959364, 0.264466415, 0.26274335, 0.267544474 });
	// 	const Eigen::MatrixXd sample(TrendClustering::make_sample_for_kmeans_clustering(series));
	// 	const KMeans kmeans(KMeansInitStrategies::make_kmeanspp());
	// 	RNGImpl rng(42);
	// 	std::vector<unsigned int> assignments;
	// 	const unsigned int k = kmeans.clusterise(sample, rng, assignments);
	// 	ASSERT_EQ(2u, k);
	// }

	TEST(TrendClusteringTest, calculate_derivatives_forward) {
		const std::vector<double> series({ 0, 1, 3 });
		Eigen::MatrixXd sample(TrendClustering::calculate_derivatives(series, 1, 1, true));
		ASSERT_EQ(1, sample.rows());
		ASSERT_EQ(3, sample.cols());
		ASSERT_EQ(1., sample(0, 0));
		ASSERT_EQ(2., sample(0, 1));
		ASSERT_EQ(2., sample(0, 2));
	}

	TEST(TrendClusteringTest, calculate_derivatives_backward) {
		const std::vector<double> series({ 0, 1, 3 });
		Eigen::MatrixXd sample(TrendClustering::calculate_derivatives(series, 1, 1, false));
		ASSERT_EQ(1, sample.rows());
		ASSERT_EQ(3, sample.cols());
		ASSERT_EQ(1., sample(0, 0));
		ASSERT_EQ(1., sample(0, 1));
		ASSERT_EQ(2., sample(0, 2));
	}

	TEST(TrendClusteringTest, map_clusters_in_trends) {
		const unsigned int k = 2;
		const std::vector<unsigned int> assignments({ 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
		std::vector<size_t> boundaries;
		std::vector<unsigned int> trends;
		const std::vector<size_t> nbr_err = TrendClustering::map_clusters_in_trends(assignments, k, trends, boundaries);
		ASSERT_EQ(std::vector<unsigned int>({ 1, 0 }), trends);
		ASSERT_EQ(std::vector<size_t>({ 1, 1 }), nbr_err);
		ASSERT_EQ(1, boundaries.size());
		ASSERT_EQ(5, boundaries.front());
	}

	TEST(TrendClusteringTest, calc_mean_indices) {
		const unsigned int k = 2;
		const std::vector<unsigned int> assignments({ 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
		const std::vector<double> means = TrendClustering::calc_mean_indices(assignments, k);
		ASSERT_EQ(2, means.size());
		ASSERT_NEAR(12.7058823529412, means[0], 1e-10);
		ASSERT_NEAR(3., means[1], 1e-10);
	}

	TEST(TrendClusteringTest, test_internal) {
		const unsigned int k = 2;
		const std::vector<unsigned int> assignments({ 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
		const std::vector<size_t> sizes = TrendClustering::calc_cluster_sizes(assignments, k);
		ASSERT_EQ(std::vector<size_t>({ 17, 5 }), sizes);
		const std::vector<size_t> medians = TrendClustering::calc_median_indices(assignments, sizes, k);
		ASSERT_EQ(std::vector<size_t>({ 13, 2 }), medians);
		const std::vector<unsigned int> sorted_cluster_indices = TrendClustering::sort_clusters_by_median_index(medians, k);
		ASSERT_EQ(std::vector<unsigned int>({ 1, 0 }), sorted_cluster_indices);
		const std::vector<unsigned int> cluster_ranks = TrendClustering::calc_cluster_ranks(sorted_cluster_indices, k);
		ASSERT_EQ(std::vector<unsigned int>({ 1, 0 }), cluster_ranks);
	}

	TEST(TrendClusteringTest, reassign_to_trends) {
		std::vector<unsigned int> assignments({ 1, 1, 0, 2 });
		const std::vector<unsigned int> trends({1, 0, 2});
		TrendClustering::reassign_to_trends(trends, assignments);
		ASSERT_EQ(std::vector<unsigned int>({ 0, 0, 1, 2 }), assignments);
	}

	TEST(TrendClusteringTest, DownAndUp) {
		RNGImpl rng(42);
		const int n = 11;
		const int m = n / 2;
		std::vector<double> series(n);
		const double eps = 0.01;
		for (int i = 0; i < m; ++i) {
			series[i] = - (i - m) + rng.next_uniform() * eps;
		}
		for (int i = m; i < n; ++i) {
			series[i] = i - m + rng.next_uniform() * eps;
		}
		const Eigen::MatrixXd sample(TrendClustering::make_sample_for_kmeans_clustering(series));
		//Eigen::MatrixXd sample(TrendClustering::differentiate(series, 0, 1));
		//KMeans::rescale_to_01(sample);
		const KMeans kmeans(KMeansInitStrategies::make_kmeanspp());
		std::vector<unsigned int> assignments;
		const unsigned int k = kmeans.clusterise(sample, rng, assignments);
		std::vector<unsigned int> trends;
		std::vector<size_t> boundaries;
		const auto nerr = TrendClustering::map_clusters_in_trends(assignments, k, trends, boundaries);
		ASSERT_EQ(2, k) << boundaries;
		ASSERT_EQ(m, static_cast<int>(boundaries[0])) << boundaries;
		ASSERT_EQ(0u, nerr[0]);
		ASSERT_EQ(0u, nerr[1]);
	}

	TEST(TrendClusteringTest, DownAndFlat) {
		const int n = 51;
		const int m = n / 2;
		const double eps = 0.1;
		std::vector<double> series(n);
		RNGImpl rng(42);
		for (int i = 0; i < m; ++i) {
			series[i] = -(i - m) + rng.next_uniform() * eps;
		}
		for (int i = m; i < n; ++i) {
			series[i] = rng.next_uniform() * eps;
		}
		const Eigen::MatrixXd sample(TrendClustering::make_sample_for_kmeans_clustering(series));
		//Eigen::MatrixXd sample(TrendClustering::differentiate(series, 0, 1));
		//KMeans::rescale_to_01(sample);
		const KMeans kmeans(KMeansInitStrategies::make_kmeanspp());
		std::vector<unsigned int> assignments;
		const unsigned int k = kmeans.clusterise(sample, rng, assignments);
		std::vector<unsigned int> trends;
		std::vector<size_t> boundaries;
		const auto nerr = TrendClustering::map_clusters_in_trends(assignments, k, trends, boundaries);
		EXPECT_LE(2u, k) << boundaries;
	}

	TEST(TrendClusteringTest, make_sample_for_kmeans_clustering) {
		std::vector<double> series({ 1., 2., 5. });
		const Eigen::MatrixXd sample(TrendClustering::make_sample_for_kmeans_clustering(series));
		ASSERT_EQ(3, sample.cols());
		ASSERT_EQ(2, sample.rows());
		
		// dy/dx values
		ASSERT_NEAR(0.5, sample(0, 0), 1e-10);
		ASSERT_NEAR(1.5, sample(0, 1), 1e-10);
		ASSERT_NEAR(1.5, sample(0, 2), 1e-10);

		// x values
		ASSERT_NEAR(0.0, sample(1, 0), 1e-10);
		ASSERT_NEAR(0.5, sample(1, 1), 1e-10);
		ASSERT_NEAR(1.0, sample(1, 2), 1e-10);
	}
}
