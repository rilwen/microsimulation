/*
 * (C) Averisera Ltd 2014
 */
#ifndef __AVERISERA_STATISTICS_H
#define __AVERISERA_STATISTICS_H

#include "data_check_level.hpp"
#include <algorithm>
#include <cassert>
#include <vector>
#include <Eigen/Core>
#include "index_iterator.hpp"
#include "math_utils.hpp"

namespace averisera {

	struct ObservedDiscreteData;
    
	/** Various statistical functions */
    namespace Statistics {
        
        // Calculate Kendall tau for a joint distribution of X and Y, where P(X = x_k && Y = y_l) = joint_distribution[k*dimY + l]
        // V: vector-like class
        template <class V> double kendall_tau(const V& joint_distribution, const unsigned int dimX, const unsigned int dimY) {
            double sum = 0;
            for (unsigned int i1 = 0; i1 < dimX; i1++) {
                for (unsigned int j1 = 0; j1 < dimY; j1++) {
                    const double p1 = joint_distribution[i1*dimY + j1];
                    for (unsigned int i2 = 0; i2 < dimX; i2++) {
                        for (unsigned int j2 = 0; j2 < dimY; j2++) {
                            const double p2 = joint_distribution[i2*dimY + j2];
                            const int si = MathUtils::sgn((int)i1 - (int)i2);
                            const int sj = MathUtils::sgn((int)j1 - (int)j2);
                            sum += si * sj * p1 * p2;
                        }
                    }
                }
            }
            return sum;
        }
        
        // Bayesian Information Criterion
        // log_likelihood: Natural logarithm of the maximized likelihood
        // number_parameters: Number of parameters in the model
        // number_observations: Number of independent observations used to calibrate the model
        double bic(double log_likelihood, unsigned int number_parameters, unsigned int number_observations);
        
        // Bayesian Information Criterion for cross-sectional models (MLR or CSM)
        // entropy: Shannon entropy of the observed probability distributions
        // kl_div: Kullback-Leibler divergence D(observed||predicted) minimized by the model
        // number_parameters: Number of parameters in the model
        // number_observations: Number of independent observations used to calibrate the model
        inline double bic(double entropy, double kl_div, unsigned int number_parameters, unsigned int number_observations) {
            return bic(-entropy - kl_div, number_parameters, number_observations);
        }
        
        /** Akaike Information Criterion, without the finite sample size correction
         *		@param log_likelihood Natural logarithm of the maximized likelihood
         *		@param number_parameters Number of parameters in the model
         *		@return Value of AIC
         */
        double aic(double log_likelihood, unsigned int number_parameters);
        
        /** Akaike Information Criterion, without the finite sample size correction, for cross-sectional models (MLR or CSM)
         *		@param entropy Shannon entropy of the observed probability distributions
         *		@param kl_div Kullback-Leibler divergence D(observed||predicted) minimized by the model
         *		@param number_parameters Number of parameters in the model
         *		@return Value of AIC
         */
        inline double aic(double entropy, double kl_div, unsigned int number_parameters) {
            return aic(-entropy - kl_div, number_parameters);
        }
        
        /** Akaike Information Criterion, including the finite sample size correction
         *		@param log_likelihood Natural logarithm of the maximized likelihood
         *		@param number_parameters Number of parameters in the model
         *		@param number_observations Number of independent observations used to calibrate the model
         *		@return Value of AICc
         */
        double aic_corr(double log_likelihood, unsigned int number_parameters, unsigned int number_observations);
        
        /** Akaike Information Criterion, including the finite sample size correction, for cross-sectional models (MLR or CSM)
         *		@param entropy Shannon entropy of the observed probability distributions
         *		@param kl_div Kullback-Leibler divergence D(observed||predicted) minimized by the model
         *		@param number_parameters Number of parameters in the model
         *		@param number_observations Number of independent observations used to calibrate the model
         *		@return Value of AIC
         */
        inline double aic_corr(double entropy, double kl_div, unsigned int number_parameters, unsigned int number_observations) {
            return aic_corr(-entropy - kl_div, number_parameters, number_observations);
        }
        
        // Calculate Kullback-Leibler divergence D(P || Q)
        // V1 and V2 are vector-like classes or arrays
        // P: "true" or "reference" distribution
        // Q: "perturbed" or "erroneous" distribution
        // n: Size of the array
        template <class V1, class V2, class Scalar = double> Scalar kl_divergence(const V1& P, const V2& Q, const size_t n)
        {
            Scalar divergence = 0;
            for (size_t i = 0; i < n; ++i) {
                const Scalar p_i = P[i];
                const Scalar q_i = Q[i];
                
                /*assert(p_i >= -1E-15);
                 *				assert(q_i >= -1E-15);
                 *				assert(p_i <= 1 + 1E-15);
                 *				assert(q_i <= 1 + 1E-15);*/
                
                if (p_i != 0.0) {
                    divergence += p_i * log(p_i / q_i);
                }
            }
            return divergence;
        }
        
        // Calculate Kullback-Leibler divergence D(P || Q)
        // V1 and V2 are vector-like classes, e.g. std::vector or Eigen::VectorXd
        // P: "true" or "reference" distribution
        // Q: "perturbed" or "erroneous" distribution
        template <class V1, class V2, class Scalar = double> Scalar kl_divergence(const V1& P, const V2& Q)
        {
            const size_t n = P.size();
            assert(n == static_cast<size_t>(Q.size()));
            return kl_divergence<V1, V2, Scalar>(P, Q, n);
        }
        
        // K-L divergence weighted by number of samples
        template <class V1, class V2, class Scalar> Scalar weighted_kl_divergence(Scalar nsamples, const V1& P, const V2& Q)
        {
            const size_t n = P.size();
            assert(n == static_cast<size_t>(Q.size()));
            return nsamples * kl_divergence<V1, V2, Scalar>(P, Q, n);
        }
        
        
        
        // Calculate the value and gradient of Kullback-Leibler divergence D(P || Q) over Q
        // V1, V2 and V3 are vector-like classes, e.g. std::vector or Eigen::VectorXd
        // P: "true" or "reference" distribution
        // Q: "perturbed" or "erroneous" distribution
        // grad: Gradient
        template <class V1, class V2, class V3> double kl_divergence_and_gradient(const V1& P, const V2& Q, V3& grad)
        {
            const size_t n = P.size();
            assert(n == Q.size());
            
            double divergence = 0;
            for (size_t i = 0; i < n; ++i) {
                const double p_i = P[i];
                const double q_i = Q[i];
                
                assert(p_i >= -1E-15);
                assert(q_i >= -1E-15);
                assert(p_i <= 1 + 1E-15);
                assert(q_i <= 1 + 1E-15);
                
                if (p_i != 0) {
                    divergence += p_i * log(p_i / q_i);
                    grad[i] = -p_i / q_i;
                } else {
                    grad[i] = 0.0;
                }
            }
            return std::max(0.0, divergence);
        }
        
        // V: vector-like class
        // Scalar: scalar type
        // P: probability distribution
        template <class V, class Scalar = double> Scalar shannon(const V& P) {
            const size_t n = P.size();
            Scalar entropy(0.0);
            for (size_t i = 0; i < n; ++i) {
                const Scalar p_i = P[i];
                if (p_i != 0.0) {
                    entropy -= p_i * log(p_i);
                }
            }
            return entropy;
        }
        
        // Weighted Shannon entropies
        // M: Matrix-like class of Scalar
        // V: vector-like class of scalar type
        // Scalar: scalar type
        // Ps: Matrix with prob. distributions in columns
        // ws: Weights vector
        template <class M, class V, class Scalar = double> Scalar shannon(const M& Ps, const V& ws) {
            const size_t T = ws.size();
            const size_t n = Ps.rows();
            Scalar entropy(0.0);
            for (size_t t = 0; t < T; ++t) {
                Scalar entropy_t(0.0);
                for (size_t i = 0; i < n; ++i) {
                    const Scalar p = Ps(i, t);
                    if (p != 0.0) {
                        entropy_t -= p * log(p);
                    }
                }
                entropy += entropy_t * ws[t];
            }
            return entropy;
        }
        
        // Calculate autocorrelations from longitudinal data
		std::vector<double> autocorrelations(const ObservedDiscreteData& data);
        
        /** Calculate standard deviations of model outputs using delta method
		@param covariance Covariance matrix of model parameters.
        @param deltas Matrix of deltas (in columns)
		@param negative_variance_tolerance How much below 0 can the calculated variance fall.
		
		@throw std::domain_error If !(negative_variance_tolerance >= 0);
		@throw std::runtime_error If the provided covariance matrix leads to calculation of negative output variance in excess of provided tolerance.
		*/
        Eigen::VectorXd standard_deviations_delta(const Eigen::MatrixXd& covariance, const Eigen::MatrixXd& deltas, double negative_variance_tolerance);
        
        /** Given a sequence of values, calculate the corresponding percentiles in-place.
         * 
         *        @tparam I Iterator over the sequence
         */
        template <class I> void percentiles_inplace(const I begin, const I end) {
            const size_t n = std::distance(begin, end);
            std::vector<I> iters(n);
            typename std::vector<I>::iterator it2 = iters.begin();
            for (I it = begin; it != end; ++it, ++it2) {
                assert(it2 != iters.end());
                *it2 = it;
            }
            std::sort(iters.begin(), iters.end(), [](I a, I b) { return *a < *b; });
            size_t idx = 0;
            for (auto pit = iters.begin(); pit != iters.end(); ++pit, ++idx) {
                *(*pit) = (static_cast<double>(idx) + 0.5) / static_cast<double>(n);
            }
        }
        
        /** Convert multidimensional values to percentiles of their marginal distributions.
          @tparam Eigen matrix-like type
         * @param[in,out] data Matrix with sample points in rows.
         */
        template <class M> void percentiles_inplace(M& data) {
            const unsigned int nc = static_cast<unsigned int>(data.cols());
            for (unsigned int c = 0; c < nc; ++c) {
                auto col = data.col(c);
                auto it = make_index_iterator_begin(col);
                const auto end = make_index_iterator_end(col);
                percentiles_inplace(it, end);
            }
        }

        /** Estimate covariance matrix assuming independent samples in each row.
         * Uses an unbiased estimator with unknown mean. Does not skip NaN or infinite values.
         @param[in] data Data matrix with samples
         @param[in] check_evel What condition to apply to every row element before including it in the estimation.
         @param[out] cov Covariance matrix. Resized if needed.
		 @throw std::domain_error If data has less than 2 rows
        */
        void estimate_covariance_matrix(Eigen::Ref<const Eigen::MatrixXd> data, DataCheckLevel check_level, Eigen::MatrixXd& cov);
        
        /** Estimate covariance matrix assuming independent samples in each row.
         * Uses an unbiased estimator with known mean, hence divides by N not by N-1.  Does not skip NaN or infinite values.
         @param[in] data Data matrix with samples
         @param[in] means Vector of known means (i.e. not calculated from data).
         @param[in] check_evel What condition to apply to every row element before including it in the estimation.
         @param[out] cov Covariance matrix. Resized if needed.
		 @throw std::domain_error If data has less than 1 row
		 @return Number of rows included in the estimation
        */
        size_t estimate_covariance_matrix(Eigen::Ref<const Eigen::MatrixXd> data, Eigen::Ref<const Eigen::VectorXd> means, DataCheckLevel check_level, Eigen::MatrixXd& cov);

        /** Check if the matrix is a good covariance matrix (doesn't test positive-semidefiniteness because it's too expensive).
          @param[in] cov Covariance matrix
          @param[out] message If not null, put an error message if needed.
          @return True if cov passes the test for a covariance matrix.
        */
        bool check_covariance_matrix(Eigen::Ref<const Eigen::MatrixXd> cov, std::string* message = nullptr);

        /** Check if the matrix is a good correlation matrix (doesn't test positive-semidefiniteness because it's too expensive).
          @param[in] rho Correlation matrix
          @param[out] message If not null, put an error message if needed.
          @return True if rho passes the test for a correlation matrix.
        */
        bool check_correlation_matrix(Eigen::Ref<const Eigen::MatrixXd> rho, std::string* message = nullptr);		

		/**
		Calculate efficiently sum of squared differences for each pair of elements: 0.5 * sum_i sum_j (v[i] - v[j])^2
		@param v input vector
		*/
		double sum_squared_differences_pairwise(const std::vector<double>& v);

		/** Calculate median by sorting the vector. If values.size() is even, median is average of two central values.
		@throw std::domain_error If values.empty() 
		*/
		template <class V> V median(std::vector<V>& values) {
			check_that(!values.empty());
			std::sort(values.begin(), values.end());
			const size_t len = values.size();
			const size_t half = len / 2;
			if (len % 2 == 0) {
				return 0.5 * values[half] + 0.5 * values[half - 1];
			} else {
				return values[half];
			}
		}
    } // namespace Statistics
} // namespace averisera

#endif
