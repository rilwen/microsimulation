#pragma once
/*
(C) Averisera Ltd 2017
*/
#include <Eigen/Core>
#include <memory>
#include <utility>
#include <vector>

namespace averisera {
	class RNG;
	template <class T> class RunningMean;

	/** K-means algorithm using Euclidean distance.
	Doesn't handle infinite point coordinates. */
	class KMeans {
	public:
		///** Measures distance between sample points */
		//class Distance {
		//public:
		//	virtual ~Distance() {}

		//	/** Measure distance from p1 to p2 */
		//	virtual double operator()(Eigen::Ref<const Eigen::VectorXd> p1, Eigen::Ref<const Eigen::VectorXd> p2) const = 0;
		//};
		
		/** Chooses initial cluster locations */
		class InitStrategy {
		public:
			virtual ~InitStrategy() {}

			/** Calculate initial cluster locations 
			@param sample d x n matrix with a sample point in each column
			@param k Number of clusters
			@param rng Random number generator
			@return d x k matrix with a cluster centroid in each column
			@throw std::domain_error if k >= n or k == 0
			*/
			virtual Eigen::MatrixXd initialise(const Eigen::MatrixXd& sample, const unsigned int k, RNG& rng) const = 0;
		};

		/**
		@param init_strat Initialisation strategy
		@param tol_abs Stop if average distance between current and previous centroids is less than this amount
		@param tol_rel Stop if average distance between current and previous centroids changes by less than this fraction of previous distance
		@param ref_pca Use PCA to calculate reference distribution sample
		@param max_iterations Maximum number of cluster assignments
		@param b Number of samples used to estimate reference gap statistics
		@throw std::domain_error If init_strat is null, tol_abs < 0 or tol_rel < 0 or max_iterations == 0 or b == 0
		*/
		KMeans(std::unique_ptr<const InitStrategy>&& init_strat, double tol_abs = 1e-6, double tol_rel = 1e-6,
			bool ref_pca = true, unsigned int max_iterations = 1000, unsigned int b = 300);

		/** Perform k-means clustering on the sample 
		@param sample d x n matrix with a sample point in each column
		@param k Number of clusters
		@param rng Random number generator
		@return n-vector of cluster indices 
		@throw std::domain_error If k == 0 or k >= n. 
		*/
		std::vector<unsigned int> clusterise(const Eigen::MatrixXd& sample, unsigned int k, RNG& rng) const {
			std::vector<unsigned int> assignments;
			clusterise(sample, k, rng, assignments);
			return assignments;
		}

		/** Perform k-means clustering on the sample
		@param sample d x n matrix with a sample point in each column
		@param k Number of clusters
		@param rng Random number generator
		@param[out] assignments Resized to length n
		@throw std::domain_error If k == 0 or k >= n.
		*/
		void clusterise(const Eigen::MatrixXd& sample, unsigned int k, RNG& rng, std::vector<unsigned int>& assignments) const;

		/** Perform k-means clustering on the sample, detecting the number of clusters using gap statistic.
		@param sample d x n matrix with a sample point in each column
		@param rng Random number generator
		@param[out] assignments Resized to length n
		@return Number of clusters
		*/
		unsigned int clusterise(const Eigen::MatrixXd& sample, RNG& rng, std::vector<unsigned int>& assignments) const;

		/** Perform k-means clustering on the sample, detecting the number of clusters using gap statistic. Bootstrap the
		distribution of cluster number by resampling the data n_boot times.
		@param sample d x n matrix with a sample point in each column
		@param[in] n_boot Number of bootstrap iterations
		@param rng Random number generator
		@param[out] assignments Resized to length n
		@param[out] k_distr Probability distribution for k from k=1 to k=k_max (k_max = k_distr.size())
		@return Number of clusters
		*/
		unsigned int clusterise_bootstrapping(const Eigen::MatrixXd& sample, unsigned int n_boot, RNG& rng, std::vector<unsigned int>& assignments, std::vector<double>& k_distr) const;

		/** Calculate the location of centroids 
		If no sample points belong to the j-th cluster, the j-th centroid is not changed.
		@param sample d x n matrix with a sample point in each column
		@param assignments n-vector of cluster indices 
		@param centroids d x k matrix with a cluster centroid in each column 
		@throw std::domain_error If k == 0 or k >= n. 
		*/
		static void update_centroids(const Eigen::MatrixXd& sample, const std::vector<unsigned int>& assignments, unsigned int k, Eigen::MatrixXd& centroids);

		/** Assign the nearest centroid to every sample point. Takes k first centroids into account.
		@param sample d x n matrix with a sample point in each column
		@param centroids d x k2 matrix with a cluster centroid in each column
		@param assignments n - vector of cluster indices
		@param rng RNG used to randomly select minimum element in case of equal distances
		@throw std::domain_error If k == 0 or k >= n or k > k2. 
		*/
		static void assign(const Eigen::MatrixXd& sample, const Eigen::MatrixXd& centroids, unsigned int k, std::vector<unsigned int>& assignments, RNG& rng);

		/** Rescale each dimension by its standard deviation 
		@param[in,out] sample d x n matrix with a sample point in each column
		*/
		static void rescale_by_standard_deviation(Eigen::MatrixXd& sample);

		/** Rescale each dimension (row) into a [0, 1] range
		@param[in,out] sample d x n matrix with a sample point in each column
		*/
		static void rescale_to_01(Eigen::MatrixXd& sample);

		/** Rescale each dimension by a provided factor
		@param[in,out] sample d x n matrix with a sample point in each column
		@param[in] factors d-length vector
		*/
		static void rescale(Eigen::MatrixXd& sample, const std::vector<double>& factors);

		/** Pooled within-cluster sum of squares (see R. Tibshirani, G. Walther and T. Hastie, "Estimating the number of clusters in a data set via the gap statistic")
		@param sample d x n matrix with a sample point in each column
		@param assignments n-vector of cluster indices
		@param k Number of clusters
		@throw std::domain_error If assignments.size() != sample.cols()
		*/
		static double pooled_within_cluster_ssq(const Eigen::MatrixXd& sample, const std::vector<unsigned int>& assignments, unsigned int k);

		/** Sample reference data (n points) from a d-dimensional box with a (0, 0, ..., 0) corner in "origin" and edges given in "edges".
		@param[in] origin Corner of the box (length d)
		@param[in] box d x d matrix with a box edge vector in each column
		@param[in] n Desired number of samples
		@param[in,out] rng Random number generator
		@param[out] ref_sample Resized to d x n shape
		@throw std::domain_error If box is not d x d
		*/
		static void sample_reference(const Eigen::VectorXd& origin, const Eigen::MatrixXd& edges, size_t n, RNG& rng, Eigen::MatrixXd& ref_sample);

		typedef std::pair<double, double> statistic_type; /**< pair of (mean, standard error) */

		/** Calculate gap statistics (see R. Tibshirani, G. Walther and T. Hastie, "Estimating the number of clusters in a data set via the gap statistic") using b reference distribution samples 
		@param sample d x n matrix with a sample point in each column
		@param assignments n-vector of cluster indices
		@param k Number of clusters
		@return pair of (mean, standard error)
		@throw std::domain_error If k == 0
		*/
		statistic_type estimate_gap_statistic(const Eigen::MatrixXd& sample, const std::vector<unsigned int>& assignments, const Eigen::VectorXd& ref_origin, const Eigen::MatrixXd& ref_edges, unsigned int k, RNG& rng) const;

		/** Accept or reject k + 1 over k given the values of their gap statistics */
		static bool accept_higher_k(const statistic_type& k_stat, const statistic_type& kp1_stat);

		/** Calculate a box encapsulating the sample, with edges along the coordinate axes */
		static void calculate_reference_box_naive(const Eigen::MatrixXd& sample, Eigen::VectorXd& origin, Eigen::MatrixXd& edges);

		/** Calculate a box encapsulating the sample, with edges along the principal axes of the data (eigenvectors of the covariance matrix) */
		static void calculate_reference_box_pca(const Eigen::MatrixXd& sample, Eigen::VectorXd& origin, Eigen::MatrixXd& edges);
	private:
		//std::unique_ptr<const Distance> dist_;
		std::unique_ptr<const InitStrategy> init_strat_;
		double tol_abs_;
		double tol_rel_;
		double max_iter_;
		unsigned int b_; /**< Number of samples used to estimate reference gap statistics */
		bool ref_pca_; /**< Use PCA to generate reference distribution */
		double gap_statistic_standard_deviation_multiplier_;

		static void update_centroids(const Eigen::MatrixXd& sample, const std::vector<unsigned int>& assignments, unsigned int k, Eigen::MatrixXd& centroids, std::vector<RunningMean<double>>& means);
	};
}
