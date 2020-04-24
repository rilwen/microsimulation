/*
(C) Averisera Ltd 2017
*/
#include <gtest/gtest.h>
#include "core/kmeans.hpp"
#include "core/kmeans_init_strats.hpp"
#include "core/rng_impl.hpp"
#include "core/running_statistics.hpp"
#include "core/stl_utils.hpp"

using namespace averisera;

void test_two_gaussians(std::unique_ptr<const KMeans::InitStrategy>&& init_strat, bool ref_pca) {
	RNGImpl rng(42);
	const double x0 = -1;
	const double y0 = -1;
	const double x1 = 1;
	const double y1 = 1;
	const double s0 = 0.01;
	const double s1 = 0.2;
	const size_t n0 = 20;
	const size_t n1 = 200;
	const size_t n = n0 + n1;

	const unsigned int d = 2;
	Eigen::MatrixXd sample(d, n);
	for (size_t i = 0; i < n0; ++i) {
		sample(0, i) = x0 + s0 * rng.next_gaussian();
		sample(1, i) = y0 + s0 * rng.next_gaussian();
	}
	for (size_t i = n0; i < n; ++i) {
		sample(0, i) = x1 + s1 * rng.next_gaussian();
		sample(1, i) = y1 + s1 * rng.next_gaussian();
	}

	unsigned int k = 2;

	Eigen::MatrixXd centroids(init_strat->initialise(sample, k, rng));
	ASSERT_EQ(k, centroids.cols());
	ASSERT_EQ(d, centroids.rows());
	for (unsigned int j = 0; j < k; ++j) {
		for (unsigned int j2 = 0; j2 < j; ++j2) {
			ASSERT_NE((centroids.col(j) - centroids.col(j2)).norm(), 0.0) << centroids;
		}
	}

	const KMeans kmeans(std::move(init_strat), 1e-8, 1e-8, ref_pca);
	
	std::vector<unsigned int> assignments(kmeans.clusterise(sample, k, rng));
	ASSERT_EQ(n, assignments.size()); 
	std::vector<RunningMean<double>> rs(k);
	for (size_t i = 0; i < n; ++i) {
		const unsigned int rs_idx = i < n0 ? 0 : 1;
		rs[rs_idx].add(static_cast<double>(assignments[i]));
	}
	double m0 = rs[0].mean();
	double m1 = rs[1].mean();
	ASSERT_LE(m0, 1);
	ASSERT_LE(m1, 1);
	ASSERT_GE(m0, 0);
	ASSERT_GE(m1, 0);
	ASSERT_NEAR(std::abs(m1 - m0), 1.0, 1e-2) << m0 << " " << m1;
	unsigned int j0 = static_cast<unsigned int>(std::round(m0));
	unsigned int j1 = static_cast<unsigned int>(std::round(m1));
	KMeans::update_centroids(sample, assignments, k, centroids);
	ASSERT_NEAR(x0, centroids(0, j0), 1e-1) << centroids;
	ASSERT_NEAR(y0, centroids(1, j0), 1e-1) << centroids;
	ASSERT_NEAR(x1, centroids(0, j1), 1e-1) << centroids;
	ASSERT_NEAR(y1, centroids(1, j1), 1e-1) << centroids;

	Eigen::VectorXd ref_origin;
	Eigen::MatrixXd ref_edges;
	if (ref_pca) {
		KMeans::calculate_reference_box_pca(sample, ref_origin, ref_edges);
	} else {
		KMeans::calculate_reference_box_naive(sample, ref_origin, ref_edges);
	}
	ASSERT_EQ(d, ref_origin.size());

	const auto gap2 = kmeans.estimate_gap_statistic(sample, assignments, ref_origin, ref_edges, k, rng);

	k = 1;
	assignments = kmeans.clusterise(sample, k, rng);
	const auto gap1 = kmeans.estimate_gap_statistic(sample, assignments, ref_origin, ref_edges, k, rng);

	ASSERT_TRUE(KMeans::accept_higher_k(gap1, gap2)) << gap1 << " " << gap2;

	k = 3;
	assignments = kmeans.clusterise(sample, k, rng);
	const auto gap3 = kmeans.estimate_gap_statistic(sample, assignments, ref_origin, ref_edges, k, rng);

	ASSERT_FALSE(KMeans::accept_higher_k(gap2, gap3)) << gap2 << " " << gap3;

	const auto detected_k = kmeans.clusterise(sample, rng, assignments);
	ASSERT_EQ(n, assignments.size());
	ASSERT_EQ(2, detected_k);
	ASSERT_EQ(1, *std::max_element(assignments.begin(), assignments.end()));
	std::for_each(rs.begin(), rs.end(), [](RunningMean<double>& r) { r.reset();  });
	for (size_t i = 0; i < n; ++i) {
		const unsigned int rs_idx = i < n0 ? 0 : 1;
		rs[rs_idx].add(static_cast<double>(assignments[i]));
	}
	m0 = rs[0].mean();
	m1 = rs[1].mean();
	ASSERT_LE(m0, 1);
	ASSERT_LE(m1, 1);
	ASSERT_GE(m0, 0);
	ASSERT_GE(m1, 0);
	ASSERT_NEAR(std::abs(m1 - m0), 1.0, 1e-2) << m0 << " " << m1;
	j0 = static_cast<unsigned int>(std::round(m0));
	j1 = static_cast<unsigned int>(std::round(m1));
	KMeans::update_centroids(sample, assignments, detected_k, centroids);
	ASSERT_NEAR(x0, centroids(0, j0), 1e-1) << centroids;
	ASSERT_NEAR(y0, centroids(1, j0), 1e-1) << centroids;
	ASSERT_NEAR(x1, centroids(0, j1), 1e-1) << centroids;
	ASSERT_NEAR(y1, centroids(1, j1), 1e-1) << centroids;
}

TEST(KMeans, TwoGaussiansForgy) {
	test_two_gaussians(KMeansInitStrategies::make_forgy(), false);
}

TEST(KMeans, TwoGaussiansRandomPartition) {
	test_two_gaussians(KMeansInitStrategies::make_random_partition(), false);
}

TEST(KMeans, TwoGaussiansRandomKMeanspp) {
	test_two_gaussians(KMeansInitStrategies::make_kmeanspp(), false);
}

TEST(KMeans, TwoGaussiansRandomKMeansppPCA) {
	test_two_gaussians(KMeansInitStrategies::make_kmeanspp(), true);
}

TEST(KMeans, RescaleByStdDev) {
	RNGImpl rng(42);
	const double sx = 2;
	const double sy = 0.1;
	const unsigned int n = 60000;
	Eigen::MatrixXd sample(2, n);
	for (size_t i = 0; i < n; ++i) {
		sample(0, i) = sx * rng.next_gaussian();
		sample(1, i) = sy * rng.next_gaussian();
	}
	const Eigen::MatrixXd orig_sample(sample);
	KMeans::rescale_by_standard_deviation(sample);
	for (size_t i = 0; i < n; ++i) {
		ASSERT_NEAR(orig_sample(0, i) / sx, sample(0, i), 1e-2) << sample.col(i);
		ASSERT_NEAR(orig_sample(1, i) / sy, sample(1, i), 1e-2) << sample.col(i);
	}
}

TEST(KMeans, pooled_within_cluster_ssq) {
	Eigen::MatrixXd sample(2, 5);
	sample << -1, -1.1, -1.2, 0.6, 0.65,
		-0.2, -0.19, -0.21, 10, 11;
	const std::vector<unsigned int> assignments({ 0, 0, 0, 1, 1});
	const double w = KMeans::pooled_within_cluster_ssq(sample, assignments, 2);
	const double expected_w = (0.1*0.1 + 0.2*0.2 + 0.1*0.1 + 0.01*0.01 + 0.01*0.01 + 0.02*0.02) / 3.0 +
		(0.05*0.05 + 1) / 2.0;
	ASSERT_NEAR(expected_w, w, 1e-10);
}

TEST(KMeans, sample_reference) {
	const unsigned int d = 2;
	const unsigned int n = 5;
	RNGImpl rng(42);
	Eigen::MatrixXd ref_sample;
	Eigen::VectorXd origin(d);
	Eigen::MatrixXd edges(d, d);
	origin << 0.5, 1.0;
	edges << 1, 0,
		0, 2;
	KMeans::sample_reference(origin, edges, n, rng, ref_sample);
	ASSERT_EQ(n, ref_sample.cols());
	ASSERT_EQ(d, ref_sample.rows());
	for (unsigned int i = 0; i < n; ++i) {
		for (unsigned int j = 0; j < d; ++j) {
			ASSERT_GE(ref_sample(j, i), origin[j]) << ref_sample.col(i);
			ASSERT_LE(ref_sample(j, i), origin[j] + edges(j, j)) << ref_sample.col(i);
		}	
	}
}

TEST(KMeans, Bootstrap) {
	const unsigned int d = 2;
	RNGImpl rng(42);
	const double x0 = -1;
	const double y0 = -1;
	const double x1 = 1;
	const double y1 = 1;
	const double s0 = 0.01;
	const double s1 = 0.05;
	const size_t n0 = 20;
	const size_t n1 = 20;
	const size_t n = n0 + n1;

	Eigen::MatrixXd sample(d, n);
	for (size_t i = 0; i < n0; ++i) {
		sample(0, i) = x0 + s0 * rng.next_gaussian();
		sample(1, i) = y0 + s0 * rng.next_gaussian();
	}
	for (size_t i = n0; i < n; ++i) {
		sample(0, i) = x1 + s1 * rng.next_gaussian();
		sample(1, i) = y1 + s1 * rng.next_gaussian();
	}
	std::vector<double> k_distr;
	const KMeans kmeans(KMeansInitStrategies::make_kmeanspp(), 1e-6, 1e-6, true);
	std::vector<unsigned int> assignments;
	const auto k = kmeans.clusterise_bootstrapping(sample, 50, rng, assignments, k_distr);
	ASSERT_EQ(2, k) << k_distr;
	ASSERT_GE(k_distr[1], k_distr[0]) << k_distr;
}

TEST(KMeans, calculate_reference_box_pca) {
	const unsigned int n = 100;
	const unsigned int d = 2;
	Eigen::MatrixXd sample(d, n);
	for (unsigned int i = 0; i < n; ++i) {
		const double dx = 0.01 * static_cast<double>(i);
		sample(0, i) = -1 + dx;
		sample(1, i) = 2 - 5 * dx;
	}
	Eigen::VectorXd origin;
	Eigen::MatrixXd edges;
	KMeans::calculate_reference_box_pca(sample, origin, edges);
	ASSERT_EQ(d, origin.size()) << origin;
	ASSERT_EQ(d, edges.rows()) << edges;
	ASSERT_EQ(d, edges.cols()) << edges;
	ASSERT_NEAR(0, edges.col(0).dot(edges.col(1)), 1e-10) << edges;
	std::vector<double> edge_norms(d);
	for (unsigned int j = 0; j < d; ++j) {
		edge_norms[j] = edges.col(j).norm();
	}
	std::sort(edge_norms.begin(), edge_norms.end());
	ASSERT_NEAR(0., edge_norms[0], 1e-10) << edges;
	ASSERT_NEAR(std::sqrt(0.99*0.99 + 4.95*4.95), edge_norms[1], 1e-10) << edges;
	std::vector<Eigen::Vector2d> corners(2);
	corners[0] = origin;
	corners[1] = origin + edges.col(0) + edges.col(1);
	std::sort(corners.begin(), corners.end(), [](const Eigen::Vector2d& l, const Eigen::Vector2d& r) {
		return l[0] < r[0];
	});
	ASSERT_NEAR(-1, corners[0][0], 1e-10);
	ASSERT_NEAR(-1e-2, corners[1][0], 1e-10);
	ASSERT_NEAR(2, corners[0][1], 1e-10);
	ASSERT_NEAR(-3 + 5e-2, corners[1][1], 1e-10);
}

TEST(KMeans, rescale_to_01) {
	const Eigen::Index n = 100;
	const Eigen::Index d = 2;
	Eigen::MatrixXd sample(d, n);
	for (Eigen::Index i = 0; i < n; ++i) {
		const double dx = 0.01 * static_cast<double>(i);
		sample(0, i) = -1 + dx;
		sample(1, i) = 2 - 5 * dx;
	}
	KMeans::rescale_to_01(sample);
	ASSERT_EQ(d, sample.rows());
	ASSERT_EQ(n, sample.cols());
	for (Eigen::Index r = 0; r < d; ++r) {
		ASSERT_NEAR(0., sample.row(r).minCoeff(), 1e-12);
		ASSERT_NEAR(1., sample.row(r).maxCoeff(), 1e-12);
	}
}
