/*
(C) Averisera Ltd 2017
*/
#include "discrete_distribution.hpp"
#include "kmeans_init_strats.hpp"
#include "preconditions.hpp"
#include "rng.hpp"

namespace averisera {
	namespace KMeansInitStrategies {
		class Forgy: public KMeans::InitStrategy {
		public:
			Eigen::MatrixXd initialise(const Eigen::MatrixXd& sample, const unsigned int k, RNG& rng) const override {
				check_not_equals(k, 0u, "KMeans::InitStrategy::initialise: k is zero");
				const size_t n = static_cast<size_t>(sample.cols());
				check_greater(n, static_cast<size_t>(k), "KMeans::InitStrategy::initialise: clusterise: k >= n");
				const size_t d = static_cast<size_t>(sample.rows());
				Eigen::MatrixXd centroids(d, k);
				std::vector<size_t> indices(k);
				// naive algorithm but should be fine for k << n, which is the regime for k-means
				for (unsigned int j = 0; j < k; ++j) {
					bool drawn_good_i = false; // "good" == not drawn before
					while (!drawn_good_i) { // will always finish because k < n
						const size_t i = static_cast<size_t>(rng.next_uniform(n - 1));
						drawn_good_i = true;
						for (unsigned int j2 = 0; j2 < j; ++j2) {
							if (i == indices[j2]) {
								drawn_good_i = false; // no, it's not a good i
							}
						}
						if (drawn_good_i) {
							indices[j] = i;
						}
					}
					centroids.col(j) = sample.col(indices[j]);
				}
				return centroids;
			}
		};

		class RandomPartition: public KMeans::InitStrategy {
		public:
			Eigen::MatrixXd initialise(const Eigen::MatrixXd& sample, const unsigned int k, RNG& rng) const override {
				check_not_equals(k, 0u, "KMeans::InitStrategy::initialise: k is zero");
				const size_t n = static_cast<size_t>(sample.cols());
				check_greater(n, static_cast<size_t>(k), "KMeans::InitStrategy::initialise: clusterise: k >= n");
				std::vector<unsigned int> assignments(n);
				for (auto& j : assignments) {
					j = static_cast<unsigned int>(rng.next_uniform(k - 1));
				}
				const size_t d = static_cast<size_t>(sample.rows());
				Eigen::MatrixXd centroids(d, k);
				centroids.setZero();
				KMeans::update_centroids(sample, assignments, k, centroids);
				return centroids;
			}
		};

		class KMeanspp : public KMeans::InitStrategy {
		public:
			Eigen::MatrixXd initialise(const Eigen::MatrixXd& sample, const unsigned int k, RNG& rng) const override {
				check_not_equals(k, 0u, "KMeans::InitStrategy::initialise: k is zero");
				const size_t n = static_cast<size_t>(sample.cols());
				check_greater(n, static_cast<size_t>(k), "KMeans::InitStrategy::initialise: clusterise: k >= n");
				const size_t d = static_cast<size_t>(sample.rows());
				Eigen::MatrixXd centroids(d, k);
				std::vector<size_t> centroid_indices(k);
				centroid_indices[0] = static_cast<size_t>(rng.next_uniform(n - 1));
				centroids.col(0) = sample.col(centroid_indices[0]);
				std::vector<unsigned int> assignments(n);
				std::vector<double> weights(n);
				for (unsigned int j = 1; j < k; ++j) {
					if (j > 1) {
						KMeans::assign(sample, centroids, j, assignments, rng);
					} else {
						std::fill(assignments.begin(), assignments.end(), 0);
					}
					double sum_weights = 0;
					for (size_t i = 0; i < n; ++i) {
						const double w = std::pow((sample.col(i) - centroids.col(assignments[i])).norm(), 2);
#ifndef NDEBUG
						check_that(std::isfinite(w), "KMeanspp: w is not finite");
#endif
						weights[i] = w;
						sum_weights += w;
					}
					double prev_cdf = 0;
					if (sum_weights == 0.0) {
						std::fill(weights.begin(), weights.end(), 1.0);
						sum_weights = static_cast<double>(n);
					}
#ifndef NDEBUG
					check_that(std::isfinite(sum_weights), "KMeanspp: sum_weights is not finite");
#endif
					for (double& w : weights) {
						w /= sum_weights;
						prev_cdf += w;
						w = prev_cdf;
					}
					weights.back() = 1.0; // now weights contains CDF values
					bool found_new_centroid = false;
					while (!found_new_centroid) { // will always finish because k < n
						const double u = rng.next_uniform();
						centroid_indices[j] = DiscreteDistribution::draw_from_cdf(weights.begin(), weights.end(), u);
						found_new_centroid = true;
						for (unsigned int j2 = 0; j2 < j; ++j2) {
							if (centroid_indices[j] == centroid_indices[j2]) {
								found_new_centroid = false;
								break;
							}
						}
					}
					centroids.col(j) = sample.col(centroid_indices[j]);
				}
				return centroids;
			}
		};

		std::unique_ptr<const KMeans::InitStrategy> make_forgy() {
			return std::make_unique<Forgy>();
		}

		std::unique_ptr<const KMeans::InitStrategy> make_random_partition() {
			return std::make_unique<RandomPartition>();
		}

		std::unique_ptr<const KMeans::InitStrategy> make_kmeanspp() {
			return std::make_unique<KMeanspp>();
		}
	}
}
