/*
(C) Averisera Ltd 2017
*/
#include "kmeans.hpp"
#include "preconditions.hpp"
#include "running_mean.hpp"
#include "trend_clustering.hpp"
#include <numeric>
#include <utility>

namespace averisera {
	template <class V1, class V2> void TrendClustering::differentiate_once_backward(const V1& input, V2& output) {
		const auto n = static_cast<size_t>(input.size());
		check_equals(n, static_cast<size_t>(output.size()), "TrendClustering::differentiate_once_backward: input/output size mismatch");
		check_not_equals(n, 0u);
		if (n > 1) {
			for (size_t i = 1; i < n; ++i) {
				output[i] = input[i] - input[i - 1];
			}
			output[0] = output[1];
		} else if (n == 1) {
			output[0] = 0;
		}
	}

	template <class V1, class V2, class V3> void TrendClustering::differentiate_once_backward(const V1& x, const V2& y, V3& output) {
		const auto n = static_cast<size_t>(x.size());
		check_equals(n, static_cast<size_t>(y.size()), "TrendClustering::differentiate_once_backward: x/y size mismatch");
		check_equals(n, static_cast<size_t>(output.size()), "TrendClustering::differentiate_once_backward: x/output size mismatch");
		check_not_equals(n, 0u);
		if (n > 1) {
			for (size_t i = 1; i < n; ++i) {
				output[i] = (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
			}
			output[0] = output[1];
		} else if (n == 1) {
			output[0] = 0;
		}
	}

	template <class V1, class V2> void TrendClustering::differentiate_once_forward(const V1& input, V2& output) {
		const auto n = static_cast<size_t>(input.size());
		check_equals(n, static_cast<size_t>(output.size()), "TrendClustering::differentiate_once_forward: input/output size mismatch");
		check_not_equals(n, 0u);
		if (n > 1) {
			for (size_t i = 0; i < n - 1; ++i) {
				output[i] = input[i + 1] - input[i];
			}
			output[n - 1] = output[n - 2];
		} else if (n == 1) {
			output[0] = 0;
		}
	}

	template <class V1, class V2, class V3> void TrendClustering::differentiate_once_forward(const V1& x, const V2& y, V3& output) {
		const auto n = static_cast<size_t>(x.size());
		check_equals(n, static_cast<size_t>(y.size()), "TrendClustering::differentiate_once_forward: x/y size mismatch");
		check_equals(n, static_cast<size_t>(output.size()), "TrendClustering::differentiate_once_forward: x/output size mismatch");
		check_not_equals(n, 0u);
		if (n > 1) {
			for (size_t i = 0; i < n - 1; ++i) {
				output[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]);
			}
			output[n - 1] = output[n - 2];
		} else if (n == 1) {
			output[0] = 0;
		}
	}

	Eigen::MatrixXd TrendClustering::calculate_derivatives(const std::vector<double>& series, const unsigned int p, const unsigned int q, const bool forward) {
		check_greater_or_equal(q, p, "TrendClustering::calculate_derivatives");
		const unsigned int d = q - p + 1;
		const size_t n = series.size();
		Eigen::MatrixXd res(d, n);
		for (size_t i = 0; i < n; ++i) {
			res(0, i) = series[i];
		}
		if (p > 0) {
			Eigen::VectorXd tmp(n);
			for (unsigned int m = 0; m < p; ++m) {
				if (forward) {
					differentiate_once_forward(res.row(0), tmp);
				} else {
					differentiate_once_backward(res.row(0), tmp);
				}
				res.row(0) = tmp;
			}
		}
		for (unsigned int m = 1; m < d; ++m) {
			auto next_row = res.row(m);
			if (forward) {
				differentiate_once_forward(res.row(m - 1), next_row);
			} else {
				differentiate_once_backward(res.row(m - 1), next_row);
			}
		}
		return res;
	}

	Eigen::MatrixXd TrendClustering::calculate_derivatives(const Eigen::MatrixXd& xy, const unsigned int p, const unsigned int q, const bool forward) {
		check_greater_or_equal(q, p, "TrendClustering::calculate_derivatives");
		check_equals(2, xy.rows(), "TrendClustering::calculate_derivatives: xy matrix must have 2 rows");
		const unsigned int d = q - p + 1;
		const auto n = static_cast<size_t>(xy.cols());
		const auto x = xy.row(0);
		const auto y = xy.row(1);
		Eigen::MatrixXd res(d, n);
		for (size_t i = 0; i < n; ++i) {
			res(0, i) = y[i];
		}
		if (p > 0) {
			Eigen::VectorXd tmp(n);
			for (unsigned int m = 0; m < p; ++m) {
				if (forward) {
					differentiate_once_forward(x, res.row(0), tmp);
				} else {
					differentiate_once_backward(x, res.row(0), tmp);
				}
				res.row(0) = tmp;
			}
		}
		for (unsigned int m = 1; m < d; ++m) {
			auto next_row = res.row(m);
			if (forward) {
				differentiate_once_forward(x, res.row(m - 1), next_row);
			} else {
				differentiate_once_backward(x, res.row(m - 1), next_row);
			}
		}
		return res;
	}

	std::vector<size_t> TrendClustering::calc_cluster_sizes(const std::vector<unsigned int>& assignments, const unsigned int k) {
		std::vector<size_t> counts(k, 0);
		for (unsigned int i : assignments) {
			++counts[i];
		}
		return counts;
	}

	std::vector<size_t> TrendClustering::calc_median_indices(const std::vector<unsigned int>& assignments, const std::vector<size_t>& cluster_sizes, const unsigned int k) {
		const size_t n = assignments.size();
		std::vector<size_t> medians(k, n);			
		std::vector<size_t> visited(k, 0);
		std::vector<size_t> half_sizes(k);
		for (unsigned int j = 0; j < k; ++j) {
			half_sizes[j] = cluster_sizes[j] / 2;
			if (cluster_sizes[j] % 2 != 0) {
				++half_sizes[j];
			}
		}
		unsigned int done = 0; // how many medians calculated so far
		for (size_t i = 0; i < n; ++i) {
			const unsigned int j = assignments[i];
			if (medians[j] == n) { // median index not found yet
				++visited[j];
				if (visited[j] == half_sizes[j]) {
					assert(medians[j] == n);
					medians[j] = i;
					++done;
					if (done == k) {
						// no more medians to calculate
						break;
					}
				}
			}
		}
		return medians;
	}

	std::vector<double> TrendClustering::calc_mean_indices(const std::vector<unsigned int>& assignments, const unsigned int k) {
		std::vector<RunningMean<double>> rms(k);
		const size_t n = assignments.size();
		for (size_t i = 0; i < n; ++i) {
			rms[assignments[i]].add(static_cast<double>(i));
		}
		std::vector<double> means(k);
		std::transform(rms.begin(), rms.end(), means.begin(), [](const RunningMean<double>& rm) { return rm.mean(); });
		return means;
	}

	std::vector<unsigned int> TrendClustering::sort_clusters_by_median_index(const std::vector<size_t>& medians, const unsigned int k) {
		std::vector<unsigned int> indices(k);
		std::iota(indices.begin(), indices.end(), 0u);
		std::sort(indices.begin(), indices.end(), [&medians](unsigned int l, unsigned int r) { 
			return medians[l] < medians[r];			
		});
		return indices;
	}

	std::vector<unsigned int> TrendClustering::calc_cluster_ranks(const std::vector<unsigned int>& cluster_indices_sorted, const unsigned int k) {
		std::vector<unsigned int> cluster_ranks(k);
		for (unsigned int j = 0; j < k; ++j) {
			cluster_ranks[cluster_indices_sorted[j]] = j;
		}
		return cluster_ranks;
	}

	void TrendClustering::initialise_boundaries(const std::vector<size_t>& cluster_sizes, const std::vector<unsigned int>& cluster_indices_sorted, const unsigned int k, std::vector<size_t> & boundaries) {
		boundaries.resize(k - 1);
		size_t prev_boundary = 0;
		for (unsigned int j = 0; j < k - 1; ++j) {
			boundaries[j] = prev_boundary + cluster_sizes[cluster_indices_sorted[j]];
			prev_boundary = boundaries[j];
		}
	}

	std::vector<size_t> TrendClustering::calc_errors_per_cluster(const std::vector<unsigned int>& assignments, const std::vector<unsigned int>& cluster_indices_sorted, const std::vector<size_t> & boundaries, const unsigned int k) {
		std::vector<size_t> error_per_cluster(k, 0);
		const size_t n = assignments.size();
		size_t prev_boundary = 0;
		for (unsigned int j = 0; j < k; ++j) {
			const size_t next_boundary = (j == k - 1) ? n : boundaries[j];
			const unsigned int target_cluster_idx = cluster_indices_sorted[j];
			size_t& counter = error_per_cluster[target_cluster_idx];
			for (size_t i = prev_boundary; i < next_boundary; ++i) {
				if (assignments[i] != target_cluster_idx) {
					++counter;
				}
			}
			prev_boundary = next_boundary;
		}
		return error_per_cluster;
	}

	std::vector<size_t> TrendClustering::map_clusters_in_trends(const std::vector<unsigned int>& assignments, const unsigned int k, std::vector<unsigned int>& trends, std::vector<size_t> & boundaries) {
		check_greater(k, 0u);
		const std::vector<size_t> cluster_sizes(calc_cluster_sizes(assignments, k));
		const std::vector<size_t> cluster_medians(calc_median_indices(assignments, cluster_sizes, k));
		const std::vector<double> cluster_means(calc_mean_indices(assignments, k));

		const std::vector<unsigned int> cluster_indices_sorted(sort_clusters_by_median_index(cluster_medians, k));
		trends = calc_cluster_ranks(cluster_indices_sorted, k);

		initialise_boundaries(cluster_sizes, cluster_indices_sorted, k, boundaries);

		return calc_errors_per_cluster(assignments, cluster_indices_sorted, boundaries, k);
	}

	Eigen::MatrixXd TrendClustering::make_sample_for_kmeans_clustering(const std::vector<double>& series) {
		check_greater_or_equal(series.size(), 2u);
		const auto n = series.size();
		Eigen::MatrixXd xy(2, n);
		for (size_t c = 0; c < n; ++c) {
			xy(0, c) = static_cast<double>(c);
			xy(1, c) = series[c];
		}
		KMeans::rescale_to_01(xy);
		Eigen::MatrixXd sample(calculate_derivatives(xy, 1, 1, true));
		const auto d = sample.rows();
		sample.conservativeResize(d + 1, sample.cols());
		sample.row(d) = xy.row(0);
		return sample;
	}
}
