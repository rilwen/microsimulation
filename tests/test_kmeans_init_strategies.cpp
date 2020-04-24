/*
(C) Averisera 2017
*/
#include <gtest/gtest.h>
#include "core/kmeans_init_strats.hpp"
#include "core/rng_impl.hpp"

using namespace averisera;

static void assert_no_duplicates(const Eigen::MatrixXd& centroids) {
	for (Eigen::Index j = 0; j < centroids.cols(); ++j) {
		for (Eigen::Index j2 = 0; j2 < j; ++j2) {
			ASSERT_NE((centroids.col(j) - centroids.col(j2)).norm(), 0);
		}
	}
}

static void assert_all_in_sample(const Eigen::MatrixXd& centroids, const Eigen::MatrixXd& sample) {
	for (Eigen::Index j = 0; j < centroids.cols(); ++j) {
		bool is_in_sample = false;
		for (Eigen::Index j2 = 0; j2 < sample.cols(); ++j2) {
			if (centroids.col(j) == sample.col(j2)) {
				is_in_sample = true;
				break;
			}
		}
		ASSERT_TRUE(is_in_sample) << j << " " << centroids.col(j);
	}
}

TEST(KMeansInitStrategies, Forgy) {
	Eigen::MatrixXd sample(2, 5);
	sample << -1, -1.1, -1.2, 0.6, 0.65,
		-0.2, -0.19, -0.21, 10, 11;
	RNGImpl rng(42);
	const auto is = KMeansInitStrategies::make_forgy();
	const unsigned int k = 3;
	const Eigen::MatrixXd centroids(is->initialise(sample, k, rng));
	ASSERT_EQ(sample.rows(), centroids.rows());
	ASSERT_EQ(k, centroids.cols());
	assert_no_duplicates(centroids);
	assert_all_in_sample(centroids, sample);
}

TEST(KMeansInitStrategies, RandomPartition) {
	Eigen::MatrixXd sample(2, 5);
	sample << -1, -1.1, -1.2, 0.6, 0.65,
		-0.2, -0.19, -0.21, 10, 11;
	RNGImpl rng(42);
	const auto is = KMeansInitStrategies::make_random_partition();
	const unsigned int k = 3;
	const Eigen::MatrixXd centroids(is->initialise(sample, k, rng));
	ASSERT_EQ(sample.rows(), centroids.rows());
	ASSERT_EQ(k, centroids.cols());
	assert_no_duplicates(centroids);
}

TEST(KMeansInitStrategies, KMeansPP) {
	Eigen::MatrixXd sample(2, 5);
	sample << -1, -1.1, -1.2, 0.6, 0.65,
		-0.2, -0.19, -0.21, 10, 11;
	RNGImpl rng(42);
	const auto is = KMeansInitStrategies::make_kmeanspp();
	const unsigned int k = 3;
	const Eigen::MatrixXd centroids(is->initialise(sample, k, rng));
	ASSERT_EQ(sample.rows(), centroids.rows());
	ASSERT_EQ(k, centroids.cols());
	assert_no_duplicates(centroids);
	assert_all_in_sample(centroids, sample);
}
