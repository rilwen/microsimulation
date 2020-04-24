/*
(C) Averisera Ltd 2015
*/
#include "log.hpp"
#include "transition_matrix_inversion.hpp"
#include <cassert>
#include <cmath>
#include <iostream>
#include <numeric>
#include <vector>
#include "nlopt_wrap.hpp"

namespace averisera {
	namespace TransitionMatrixInversion {
		struct TransitionMatrixInversionData {
			TransitionMatrixInversionData(double tolerance, const Eigen::MatrixXd& npi, const Eigen::VectorXd& ny) 
			: pi(npi), y(ny), r(ny.size()) { }

			const Eigen::MatrixXd& pi;
			const Eigen::VectorXd& y;
			Eigen::VectorXd r;
		};

		namespace {
			double optimized_function(const std::vector<double>& xdata, std::vector<double>& grad, void* f_data) {
				TransitionMatrixInversionData* tmi_data = static_cast<TransitionMatrixInversionData*>(f_data);
				assert(tmi_data);
				const unsigned int dim = static_cast<unsigned int>(xdata.size());
				Eigen::Map<const Eigen::VectorXd> x(&xdata[0], dim);
				Eigen::VectorXd& r = tmi_data->r;
				assert(x.size() == r.size());
				const Eigen::MatrixXd& pi = tmi_data->pi;
				assert(pi.rows() == r.size());
				assert(pi.cols() == r.size());
				const Eigen::VectorXd& y = tmi_data->y;
				assert(y.size() == r.size());
				r = pi * x;
				r -= y;
				if (grad.size()) {
					assert(grad.size() == static_cast<size_t>(x.size()));
					// calculate gradient over x
					Eigen::Map<Eigen::VectorXd> vg(&grad[0], dim);
					vg = pi.transpose() * r;
					vg *= 2;
				}
				return r.squaredNorm();
			}
		}

		double constraint_function(const std::vector<double>& v, std::vector<double>& grad, void* f_data) {
			if (grad.size()) {
				std::fill(grad.begin(), grad.end(), 1.0);
			}
			return std::accumulate(v.begin(), v.end(), 0.) - 1.0;
		}

		Eigen::VectorXd apply_inverse_pi(const Eigen::MatrixXd& pi, const Eigen::VectorXd& y, const double tolerance) {
			TransitionMatrixInversionData data(tolerance, pi, y);
			const unsigned int dim = static_cast<unsigned int>(y.size());
			nlopt::opt opt(nlopt::LD_SLSQP, dim);
			opt.set_min_objective(optimized_function, &data);
			opt.set_lower_bounds(std::vector<double>(dim, 0.));
			opt.set_ftol_rel(tolerance); // relative tolerance for function value
			opt.set_xtol_rel(tolerance); // relative tolerance for optimal point
			opt.set_maxeval(1000); // maximum number of function evaluations
			opt.add_equality_constraint(constraint_function, 0, 1E-14);
			double value;
			std::vector<double> xx(dim);
			double sum = 0.;
			for (unsigned int i = 0; i < dim; ++i) {
				xx[i] = y[i];
				sum += y[i];
			}
			if (sum != 1.0) {
				for (unsigned int i = 0; i < dim; ++i) {
					xx[i] /= sum;
				}
			}
			const nlopt::result result = opt.optimize(xx, value);
			if (result != nlopt::SUCCESS) {
				LOG_WARN() << "TransitionMatrixInversion: status: " << nlopt::retcodestr(result) << "\n";
			}
			Eigen::VectorXd x(dim);
			for (unsigned int i = 0; i < dim; ++i) {
				x[i] = xx[i];
			}
			return x;
		}
	}
}
