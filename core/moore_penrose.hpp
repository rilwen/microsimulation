/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_MOORE_PENROSE_H
#define __AVERISERA_MOORE_PENROSE_H

#include <Eigen/Core>

namespace averisera {
	namespace MoorePenrose {
		// Calculate Moore-Penrose pseudoinverse of matrix M, omitting all eigenvalues lambda with |lambda| <= eps.
		// inverse and M can be the same matrix
		void inverse(const Eigen::MatrixXd& M, double eps, Eigen::MatrixXd& inverse);
	}
}

#endif
