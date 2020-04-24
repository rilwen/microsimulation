/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "moore_penrose.hpp"
#include <Eigen/SVD>

namespace averisera {
	namespace MoorePenrose {
		void inverse(const Eigen::MatrixXd& M, const double eps, Eigen::MatrixXd& inverse) {
			Eigen::JacobiSVD<Eigen::MatrixXd> svd(M, Eigen::ComputeFullU | Eigen::ComputeFullV);
			const size_t nvals = std::min(M.rows(), M.cols());
			Eigen::MatrixXd invS(M.cols(), M.rows());
            invS.setZero();
			for (size_t i = 0; i < nvals; ++i) {
				const double lam = svd.singularValues()[i];
				if (std::abs(lam) <= eps) {
					invS(i, i) = 0;
				}
				else {
					invS(i, i) = 1 / lam;
				}
			}
			inverse = svd.matrixV() * invS * svd.matrixU().transpose();
		}
	}
}
