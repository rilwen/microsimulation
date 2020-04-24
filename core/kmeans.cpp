/*
(C) Averisera Ltd 2017
*/
#include "bootstrap.hpp"
#include "kmeans.hpp"
#include "log.hpp"
#include "math_utils.hpp"
#include "preconditions.hpp"
#include "rng.hpp"
#include "running_mean.hpp"
#include "running_statistics.hpp"
#include "statistics.hpp"
#include "stl_utils.hpp"
#include <Eigen/SVD>

namespace averisera {
	KMeans::KMeans(std::unique_ptr<const InitStrategy>&& init_strat, double tol_abs, double tol_rel, 
		bool ref_pca, unsigned int max_iterations, unsigned int b)
		: 
		init_strat_(std::move(init_strat)), tol_abs_(tol_abs), tol_rel_(tol_rel), 
		max_iter_(max_iterations), b_(b), ref_pca_(ref_pca)
	{
		check_not_null(init_strat_, "KMeans: init_strat is null");
		check_greater(max_iterations, 0u, "KMeans: max_iterations must be positive");
		check_greater(b, 0u, "KMeans: b must be positive");
		check_greater_or_equal(tol_abs, 0.0, "KMeans: absolute tolerance cannot be negative");
		check_greater_or_equal(tol_rel, 0.0, "KMeans: relative tolerance cannot be negative");
		gap_statistic_standard_deviation_multiplier_ = std::sqrt(1 + 1.0 / static_cast<double>(b_));
	}

	void KMeans::clusterise(const Eigen::MatrixXd& sample, const unsigned int k, RNG& rng, std::vector<unsigned int>& assignments) const {
		check_not_equals(k, 0u, "KMeans::clusterise: k is zero");
		const size_t n = static_cast<size_t>(sample.cols()); // sample size
		check_greater(n, static_cast<size_t>(k), "KMeans::clusterise: k >= n");
		assignments.resize(n);
		Eigen::MatrixXd centroids(init_strat_->initialise(sample, k, rng));		
		Eigen::MatrixXd next_centroids(centroids.rows(), centroids.cols());
		std::vector<RunningMean<double>> means;
		unsigned int iter = 0;
		double distance = 0;
		const auto d = centroids.rows();
		bool converged = false;
		while (iter < max_iter_) {
			assign(sample, centroids, k, assignments, rng);
			update_centroids(sample, assignments, k, next_centroids, means);
			double next_distance = 0;
			for (unsigned int j = 0; j < k; ++j) {
				double sqr_dist = 0;
				for (Eigen::Index q = 0; q < d; ++q) {
					sqr_dist += std::pow(centroids(q, j) - next_centroids(q, j), 2);
				}
				next_distance += std::sqrt(sqr_dist);
			}
			next_distance /= k;
#ifndef NDEBUG
			check_greater_or_equal(next_distance, 0.0, "KMeans::clusterise: next_distance >= 0");
#endif
			if (iter > 0) {
				if (std::abs(next_distance - distance) < tol_abs_ + tol_rel_ * distance) {
					converged = true;
					break;
				}
			}			
			distance = next_distance;
			centroids.swap(next_centroids);
			++iter;
		}
		LOG_TRACE() << "k=" << k << ", centr=" << centroids;
		if (!converged) {
			LOG_WARN() << "KMeans::clusterise: did not converge after " << max_iter_ << " iterations";
		}
	}

	unsigned int KMeans::clusterise(const Eigen::MatrixXd& sample, RNG& rng, std::vector<unsigned int>& assignments) const {
		unsigned int k;
		unsigned int next_k = 1;
		const size_t n = static_cast<size_t>(sample.cols()); // sample size
		assignments.resize(n);
		std::vector<unsigned int> next_assignments(n);
		std::fill(next_assignments.begin(), next_assignments.end(), 0);
		Eigen::VectorXd ref_origin;
		Eigen::MatrixXd ref_edges;
		if (ref_pca_) {
			calculate_reference_box_pca(sample, ref_origin, ref_edges);
		} else {
			calculate_reference_box_naive(sample, ref_origin, ref_edges);
		}
		assert(sample.rows() == ref_origin.size());
		assert(ref_origin.size() == ref_edges.rows());
		assert(ref_edges.rows() == ref_edges.cols());
		auto next_gap_stat = estimate_gap_statistic(sample, next_assignments, ref_origin, ref_edges, next_k, rng);
		LOG_TRACE() << "k=1, assignments=" << assignments;
		statistic_type gap_stat;
		do {
			k = next_k;
			gap_stat = next_gap_stat;
			assignments.swap(next_assignments);
			++next_k;
			clusterise(sample, next_k, rng, next_assignments);
			next_gap_stat = estimate_gap_statistic(sample, next_assignments, ref_origin, ref_edges, next_k, rng);
			LOG_TRACE() << "k=" << next_k << ", asss=" << next_assignments;
		} while (accept_higher_k(gap_stat, next_gap_stat) && next_k < n);
		return k;
	}

	unsigned int KMeans::clusterise_bootstrapping(const Eigen::MatrixXd& sample, const unsigned int n_boot, RNG& rng, std::vector<unsigned int>& assignments, std::vector<double>& k_distr) const {
		k_distr.resize(0);
		Eigen::MatrixXd sample_boot(sample.rows(), sample.cols());
		RNG::StlWrapper stl_rng(rng);
		Bootstrap<RNG::StlWrapper> bootstrap(stl_rng);
		for (unsigned int i = 0; i < n_boot; ++i) {
			bootstrap.resample_with_replacement(sample, sample_boot);
			const auto k_boot = clusterise(sample_boot, rng, assignments);
			assert(k_boot > 0);
			if (k_boot > k_distr.size()) {
				k_distr.resize(k_boot, 0.0);
			}
			k_distr[k_boot - 1] += 1.0;
		}
		const double sum = std::accumulate(k_distr.begin(), k_distr.end(), 0.0);
		std::for_each(k_distr.begin(), k_distr.end(), [sum](double &p) { p /= sum; });
		return clusterise(sample, rng, assignments);
	}

	void KMeans::assign(const Eigen::MatrixXd& sample, const Eigen::MatrixXd& centroids, const unsigned int k, std::vector<unsigned int>& assignments, RNG& rng) {
		check_not_equals(k, 0u, "KMeans::assign: k is zero");
		const auto d = sample.rows();
		check_equals(d, centroids.rows(), "KMeans::assign: row count mismatch"); // these are not the centroids you are looking for
		check_greater_or_equal(static_cast<unsigned int>(centroids.cols()), k, "KMeans::assign: not enough centroids");
		const size_t n = assignments.size();
		for (size_t i = 0; i < n; ++i) {
			double lowest_square_distance = std::numeric_limits<double>::infinity();
			unsigned int closest_j = k;
			size_t nbr_equal_distances = 0;
			for (unsigned int j = 0; j < k; ++j) {
				double sqr_dist = 0;
				for (Eigen::Index q = 0; q < d; ++q) {
					const double diff = sample(q, i) - centroids(q, j);
					sqr_dist += diff * diff;
				}
				if (sqr_dist < lowest_square_distance) {
					closest_j = j;
					lowest_square_distance = sqr_dist;
					nbr_equal_distances = 1;
				} else if (sqr_dist == lowest_square_distance) {
					assert(closest_j < k); // we assume no points or centroids at infinity
					++nbr_equal_distances;
					// switch to new element with probability 1 / nbr_values
					if (rng.flip(1 / static_cast<double>(nbr_equal_distances))) {
						closest_j = j;
					}
				}
			}
			assert(closest_j < k);
			assignments[i] = closest_j;
		}
	}

	void KMeans::update_centroids(const Eigen::MatrixXd& sample, const std::vector<unsigned int>& assignments, unsigned int k, Eigen::MatrixXd& centroids) {
		std::vector<RunningMean<double>> means;
		update_centroids(sample, assignments, k, centroids, means);
	}

	void KMeans::update_centroids(const Eigen::MatrixXd& sample, const std::vector<unsigned int>& assignments, const unsigned int k, Eigen::MatrixXd& centroids, std::vector<RunningMean<double>>& means) {
		check_not_equals(k, 0u, "KMeans::update_centroids: k is zero");
		check_equals(sample.rows(), centroids.rows(), "KMeans::update_centroids: row count mismatch"); 
		const size_t d = static_cast<size_t>(sample.rows());
		const size_t n = static_cast<size_t>(sample.cols());
		means.resize(k * d);
		std::for_each(means.begin(), means.end(), [](RunningMean<double>& rm) { rm.reset(); });
		auto ass_iter = assignments.begin();
		for (size_t i = 0; i < n; ++i, ++ass_iter) {
			const Eigen::MatrixXd::ConstColXpr point = sample.col(i);
			const auto j = *ass_iter;
			auto mean_iter = means.begin() + (j * d);
			for (size_t m = 0; m < d; ++m, ++mean_iter) {
				mean_iter->add_finite(point[m]);
			}
		}
		assert(ass_iter == assignments.end());
		auto mean_iter = means.begin();
		for (unsigned int j = 0; j < k; ++j) {
			Eigen::MatrixXd::ColXpr centroid = centroids.col(j);
			if (mean_iter->nbr_samples() > 0) {
				for (size_t m = 0; m < d; ++m, ++mean_iter) {
					centroid[m] = mean_iter->mean();
				}
			} else {
				mean_iter += d;
			}
		}
		assert(mean_iter == means.end());
	}

	void KMeans::rescale_by_standard_deviation(Eigen::MatrixXd& sample) {
		const size_t d = static_cast<size_t>(sample.rows());
		const size_t n = static_cast<size_t>(sample.cols());
		std::vector<RunningStatistics<double>> stats(d);
		for (size_t i = 0; i < n; ++i) {
			const Eigen::MatrixXd::ColXpr pt = sample.col(i);
			for (size_t j = 0; j < d; ++j) {
				stats[j].add(pt[j]);
			}
		}
		for (size_t j = 0; j < d; ++j) {
			sample.row(j) /= stats[j].standard_deviation();
		}
	}

	void KMeans::rescale_to_01(Eigen::MatrixXd& sample) {
		const size_t d = static_cast<size_t>(sample.rows());
		for (size_t j = 0; j < d; ++j) {
			const double min_x = sample.row(j).minCoeff();
			const double max_x = sample.row(j).maxCoeff();
			const double width = max_x - min_x;
			if (width > 0) {
				sample.row(j).array() -= min_x;
				sample.row(j).array() /= width;
			}
		}
	}

	void KMeans::rescale(Eigen::MatrixXd& sample, const std::vector<double>& factors) {
		const size_t d = static_cast<size_t>(sample.rows());
		check_equals(d, factors.size(), "KMeans::rescale");
		for (size_t j = 0; j < d; ++j) {
			sample.row(j) *= factors[j];
		}
	}

	double KMeans::pooled_within_cluster_ssq(const Eigen::MatrixXd& sample, const std::vector<unsigned int>& assignments, const unsigned int k) {
		const auto n = static_cast<size_t>(sample.cols());
		const auto d = static_cast<unsigned int>(sample.rows());
		check_equals(n, assignments.size(), "KMeans::pooled_within_cluster_ssq: size mismatch");
		std::vector<RunningStatistics<double>> stats(k * d);
		size_t i = 0;
		for (auto it = assignments.begin(); it != assignments.end(); ++it, ++i) {
			const Eigen::MatrixXd::ConstColXpr x = sample.col(i);
			const unsigned int j = *it;
			auto st_it = stats.begin() + (j * d);
			for (unsigned int l = 0; l < d; ++l, ++st_it) {
				st_it->add(x[l]);
			}
		}
		double sum = 0.0;
		for (auto st_it = stats.begin(); st_it != stats.end(); ++st_it) {
			const auto n_j = static_cast<double>(st_it->nbr_samples());
			if (n_j > 1) {
				sum += st_it->variance() * (n_j - 1);
			};
		}
		return sum;
	}

	void KMeans::sample_reference(const Eigen::VectorXd& origin, const Eigen::MatrixXd& edges, const size_t n, RNG& rng, Eigen::MatrixXd& ref_sample) {
		const auto d = static_cast<unsigned int>(origin.size());
		check_equals(d, edges.rows(), "KMeans::sample_reference: edges.rows() == d");
		check_equals(d, edges.cols(), "KMeans::sample_reference: edges.cols() == d");
		ref_sample.resize(d, n);		
		for (size_t i = 0; i < n; ++i) {
			Eigen::MatrixXd::ColXpr col = ref_sample.col(i);
			col = origin;
			for (unsigned int l = 0; l < d; ++l) {
				col += edges.col(l) * rng.next_uniform();
			}
		}
	}

	KMeans::statistic_type KMeans::estimate_gap_statistic(const Eigen::MatrixXd& sample, const std::vector<unsigned int>& assignments, const Eigen::VectorXd& ref_origin, const Eigen::MatrixXd& ref_edges, const unsigned int k, RNG& rng) const {
		check_greater(k, 0u, "KMeans::estimate_gap_statistic: k must be positive");
		const double model_stat = std::log(pooled_within_cluster_ssq(sample, assignments, k));
		if (std::isnan(model_stat)) {
			LOG_WARN() << "KMeans::estimate_gap_statistic: model_stat==" << model_stat;
		}
		RunningStatistics<double> ref_stat;
		Eigen::MatrixXd ref_sample;
		std::vector<unsigned int> ref_assignments;
		const auto n = static_cast<size_t>(sample.cols());
		for (unsigned int i = 0; i < b_; ++i) {
			sample_reference(ref_origin, ref_edges, n, rng, ref_sample);
			clusterise(ref_sample, k, rng, ref_assignments);
			const double w = pooled_within_cluster_ssq(ref_sample, ref_assignments, k);
			if (w == 0.) {
				LOG_WARN() << "KMeans::estimate_gap_statistic: w==" << w;
			}
			ref_stat.add(std::log(w));
		}
		const double s = gap_statistic_standard_deviation_multiplier_ * ref_stat.standard_deviation();
		const auto result = std::make_pair(ref_stat.mean() - model_stat, s);
		//std::cout << result << "\n";
		return result;
	}

	bool KMeans::accept_higher_k(const statistic_type& k_stat, const statistic_type& kp1_stat) {
		if (std::isnan(kp1_stat.first) || !std::isfinite(kp1_stat.second)) {
			LOG_WARN() << "KMeans::accept_higher_k: kp1==" << kp1_stat;
		}
		return k_stat.first < (kp1_stat.first - kp1_stat.second);
	}

	void KMeans::calculate_reference_box_naive(const Eigen::MatrixXd& sample, Eigen::VectorXd& origin, Eigen::MatrixXd& edges) {
		const auto d = static_cast<unsigned int>(sample.rows());
		origin = sample.rowwise().minCoeff();
		const Eigen::VectorXd edge_diagonal(sample.rowwise().maxCoeff() - origin);
		assert(d == static_cast<unsigned int>(origin.size()));
		edges.setZero(d, d);
		for (unsigned int j = 0; j < d; ++j) {
			edges(j, j) = edge_diagonal[j];
		}
	}

	void KMeans::calculate_reference_box_pca(const Eigen::MatrixXd& sample, Eigen::VectorXd& origin, Eigen::MatrixXd& edges) {
		const auto d = static_cast<unsigned int>(sample.rows());
		Eigen::MatrixXd cov;
		Statistics::estimate_covariance_matrix(sample.transpose(), DataCheckLevel::FINITE, cov);
		assert(d == cov.rows());
		assert(cov.rows() == cov.cols());
		Eigen::JacobiSVD<Eigen::MatrixXd> svd(cov, Eigen::ComputeFullU | Eigen::ComputeFullV);
		// svd.matrixU() has an eigenvector in each column
		// svd.matrixV() == svd.matrixU().transpose()

		// TODO: handle large sample sizes

		// convert to eigenspace
		Eigen::MatrixXd ei_sample(svd.matrixV() * sample);
		Eigen::MatrixXd ei_edges;
		Eigen::VectorXd ei_origin;
		calculate_reference_box_naive(ei_sample, ei_origin, ei_edges);
		origin.resize(d);
		edges.resize(d, d);
		origin.noalias() = svd.matrixU() * ei_origin;
		edges.noalias() = svd.matrixU() * ei_edges;
	}
}
