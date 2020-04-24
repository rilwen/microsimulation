/*
(C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_TRANSITION_MATRIX_INVERSION_H
#define __AVERISERA_TRANSITION_MATRIX_INVERSION_H

#include <Eigen/Core>

namespace averisera {
	namespace TransitionMatrixInversion {
		// Solve pi * x == y for x.
		Eigen::VectorXd apply_inverse_pi(const Eigen::MatrixXd& pi, const Eigen::VectorXd& y, double tolerance);
	}
}

#endif // __AVERISERA_TRANSITION_MATRIX_INVERSION_H