/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "ssq_divergence.hpp"
#include <cassert>
#include <Eigen/Core>
#include "moore_penrose.hpp"
#include <iostream>

namespace averisera {
	SSQDivergence::SSQDivergence(const Eigen::MatrixXd& observed, const Eigen::VectorXd& nbr_surveys) {
		const size_t T = observed.cols();
		const size_t dim = observed.rows();
		assert(static_cast<size_t>(nbr_surveys.size()) == T);
		_weights.resize(T);
		Eigen::MatrixXd covariance(dim, dim); // covariance matrix
		Eigen::VectorXd alpha(dim); // parameter vector for Dirichlet distribution
		Eigen::VectorXd S(dim); // vector of eigenmode variances
		for (unsigned int t = 0; t < T; ++t) {
			alpha.setConstant(1.0); // prior
			alpha += nbr_surveys[t] * observed.col(t); // update to posterior

			// Calculate the covariance matrix
			const double a0 = alpha.sum();
			const double denom = a0 * a0 * (a0 + 1);
			for (unsigned int r = 0; r < dim; ++r) {
				const double a_r = alpha[r];
				covariance(r, r) = a_r * (a0 - a_r) / denom;
				for (unsigned int c = 0; c < r; ++c) {
					covariance(r, c) = covariance(c, r) = -a_r * alpha[c] / denom;
				}
			}

			// Calculate the Moore-Penrose pseudoinverse of the covariance matrix. Because of constraint sum_i p_i = 1, covariance matrix has rank dim - 1 and is not invertible.
			MoorePenrose::inverse(covariance, 1E-14, _weights[t]);
		}
	}

	double SSQDivergence::operator()(size_t year_idx, double, Eigen::Ref<const Eigen::VectorXd> P, Eigen::Ref<const Eigen::VectorXd> Q) const {
		assert(year_idx < _weights.size());
		const size_t dim = P.size();
		assert(dim == static_cast<size_t>(Q.size()));
		const Eigen::MatrixXd& wm = _weights[year_idx];
		assert(dim == static_cast<size_t>(wm.rows()));
		double dist = 0;

		// Apply the weights to deviations of Q from P
		for (size_t r = 0; r < dim; ++r) {
			const double diff_r = Q[r] - P[r];
			dist += diff_r * diff_r * wm(r, r);
			for (size_t c = 0; c < r; ++c) {
				const double diff_c = Q[c] - P[c];
				dist += 2 * diff_r * diff_c * wm(r, c);
			}
		}
		return dist;
	}

	unsigned int SSQDivergence::dim() const {
		if (!_weights.empty()) {
			return static_cast<unsigned int>(_weights.front().rows());
		} else {
			return 0;
		}
	}
}
