// (C) Averisera Ltd 2014-2020
#include "akima_approximation.hpp"
#include "interpolator_impl_tanh.hpp"
#include "log.hpp"
#include "preconditions.hpp"
#include "nlopt_wrap.hpp"
#include "segment_search.hpp"
#include "sacado_scalar.hpp"
#include <cmath>
#include <iostream>

namespace averisera {
	InterpolatorImplTanh::InterpolatorImplTanh(std::vector<double>&& x, const std::vector<double>& y, double lambda)
		: InterpolatorImplPiecewise(std::move(x), true), lambda_(lambda) {
		try {
			calibrate(y);
		} catch (std::exception& e) {
			x = this->x();
			throw e;
		}
	}

	InterpolatorImplTanh::InterpolatorImplTanh(const InterpolatorImplTanh& other)
		: InterpolatorImplPiecewise(other.x(), other.left_inclusive()), nodes_(other.nodes_) {}

	double InterpolatorImplTanh::evaluate_impl(const double xx, const size_t seg_idx) const {
		const double x0 = x()[seg_idx];
		const Node& node = nodes_[seg_idx];
		if (node.ax != 0) {
			// tanh interpolation
			return node.ay * tanh(node.ax * (xx - x0) + node.bx) + node.by;
		} else {
			//std::cout << "Using linear...\n";
			// linear interpolation
			return node.ay * (xx - x0) + node.by;
		}
	}

	double InterpolatorImplTanh::evaluate_derivative(double x) const {
		return evaluate_derivative(x, find_segment(x));
	}

	double InterpolatorImplTanh::evaluate_derivative(const double xx, const size_t seg_idx) const {
		const double x0 = x()[seg_idx];
		const Node& node = nodes_[seg_idx];
		if (node.ax != 0) {
			// tanh interpolation
			return node.ay * node.ax * (1 - pow(tanh(node.ax * (xx - x0) + node.bx), 2));
		} else {
			//std::cout << "Using linear...\n";
			// linear interpolation
			return node.ay;
		}
	}

	std::shared_ptr<InterpolatorImpl> InterpolatorImplTanh::clone() const {
		return std::shared_ptr<InterpolatorImpl>(new InterpolatorImplTanh(*this));
	}

	InterpolatorImpl& InterpolatorImplTanh::operator+=(double x) {
		for (Node& node : nodes_) {
			node.by += x;
		}
		return *this;
	}

	InterpolatorImpl& InterpolatorImplTanh::operator-=(double x) {
		for (Node& node : nodes_) {
			node.by -= x;
		}
		return *this;
	}

	InterpolatorImpl& InterpolatorImplTanh::operator*=(double x) {
		for (Node& node : nodes_) {
			node.ay *= x;
			node.by *= x;
		}
		return *this;
	}

	InterpolatorImpl& InterpolatorImplTanh::operator/=(double x) {
		for (Node& node : nodes_) {
			node.ay /= x;
			node.by /= x;
		}
		return *this;
	}

	static void set_linear(const double y0, const double slope, double& ax, double& bx, double& ay, double& by) {
		ax = 0;
		bx = 0;
		// solve equations ay * (x0 - x0) + by == y0 and ay * (x1 - x0) + by == y1
		// => by := y0 and ay := (y1 - y0) / (x1 - x0)
		ay = slope;
		by = y0;
	}

	void InterpolatorImplTanh::calibrate(const std::vector<double>& y) {
		check_equals(x().size(), y.size());
		//check_that(left_dydx != 0);
		//check_that(right_dydx != 0);
		//check_that(std::isfinite(left_dydx));
		//check_that(std::isfinite(right_dydx));
		//check_equals(std::signbit(left_dydx), std::signbit(right_dydx));
		//check_equals(std::signbit(left_dydx), std::signbit(y[1] - y[0]));
		//check_equals(std::signbit(right_dydx), std::signbit(y.back() - y[y.size()-2]));
		nodes_.resize(nbr_segments());
		if (nodes_.empty()) {
			return;
		}
		std::vector<double> dydx(y.size());		
		if (y.size() > 2) {
			AkimaApproximation<double>::calculate(x(), y, dydx);
			// use linear interp. at the ends to make life easier
			dydx[0] = dydx[1];
			dydx[y.size() - 1] = dydx[y.size() - 2];
		} else {
			const double slope = (y[1] - y[0]) / (x()[1] - x()[0]);
			dydx[0] = slope;
			dydx[1] = slope;
		}
		for (size_t i = 0; i < nbr_segments(); ++i) {
			const double y0 = y[i];
			const double y1 = y[i + 1];
			const double x0 = x()[i];
			const double x1 = x()[i + 1];
			const double dy = y1 - y0;
			check_that(dy != 0);
			//check_equals(std::signbit(left_dydx), std::signbit(dy));
			const double dx = x1 - x0;
			const double slope = dy / dx;
			const double dydx0 = dydx[i];
			const double dydx1 = dydx[i + 1];
			/*const double dydx0 = i == 0 ? 
				1.0 : 
				(y1 - y[i-1]) / (x1 - x()[i - 1]);
			const double dydx1 = i == (nbr_segments() - 1) ?
				1.0 :
				(y[i + 2] - y0) / (x()[i + 2] - x0);*/
			assert(dydx0 != 0);
			assert(dydx1 != 0);
			//std::cout << "dydx0==" << dydx0 << ", dydx1==" << dydx1 << std::endl;
			assert(std::signbit(dydx0) == std::signbit(dydx1));
			Node& node = nodes_[i];
			//std::cout << "Data: " << x0 << " " << x1 << " " << dydx0 << " " << dydx1 << " " << (dydx1 - dydx0) << "\n";
			const bool try_tanh = std::min(std::abs(dydx0), std::abs(dydx1)) > 1e-12 * std::abs(slope);
			//double norm = 0;
			if (try_tanh) {
				// use tanh interpolation				
				calibrate_tanh_node(lambda_, dx, y0, y1, dydx0, dydx1, node.ax, node.bx, node.ay, node.by);
			} else {
				set_linear(y0, slope, node.ax, node.bx, node.ay, node.by);
			}
			//if (!try_tanh || norm > 1e-4 * (y0*y0 + y1*y1 + dydx0*dydx0 + dydx1*dydx1)) {
			//	// use linear interpolation
			//	if (try_tanh) {
			//		std::cerr << "Falling back to linear interpolation\n";
			//	}
			//	set_linear(y0, slope, node.ax, node.bx, node.ay, node.by);
			//}
		}
	}

	namespace {
		struct ObjFunData {
			double dx;
			double y0;
			double y1;
			double dydx0;
			double dydx1;
			double lambda;
		};

		static const size_t DIM = 2;
	}

	static double objective_function(const std::vector<double>& x, std::vector<double>& grad, void* f_data) {
		const ObjFunData* data = static_cast<const ObjFunData*>(f_data);
		assert(data);		
		assert(x.size() == DIM); // ax, bx, ay, by
		unsigned int idx = 0;
		const adouble ax(from_double<0>(DIM, idx, x[0]));
		const adouble bx(from_double<0>(DIM, idx, x[1]));
		//const adouble ay(from_double<0>(DIM, idx, x[2]));
		//const adouble by(from_double<0>(DIM, idx, x[2]));
		const adouble t0(tanh(bx));
		const adouble t1(tanh(ax * data->dx + bx));
		const adouble ay = (data->y1 - data->y0) / (t1 - t0);
		const adouble by = data->y0 - ay * t0;
		const adouble y0 = ay * t0 + by;
		const adouble y1 = ay * t1 + by;
		const adouble dydx0 = ay * ax * (1 - t0 * t0);
		const adouble dydx1 = ay * ax * (1 - t1 * t1);
		adouble result = pow(y0 - data->y0, 2);
		result += pow(y1 - data->y1, 2);
		result += data->lambda * pow((dydx0 - data->dydx0) * data->dx, 2);
		result += data->lambda * pow((dydx1 - data->dydx1) * data->dx, 2);
		if (!grad.empty()) {
			assert(grad.size() == DIM);
			for (size_t i = 0; i < DIM; ++i) {
				grad[i] = result.dx(static_cast<int>(i));
			}
		}
		return result.val();
	}

	double InterpolatorImplTanh::calibrate_tanh_node(const double lambda, const double dx, const double y0, const double y1, const double dydx0, const double dydx1, double& ax, double& bx, double& ay, double& by) {
		// Need to fix 4 numbers (ax, bx, ay, by) with 4 data points (y0, y1, dydx0, dydx1)
		assert(y0 != y1);
		//assert(dydx0 != dydx1);
		assert(dx > 0);
		if (y0 < y1) {			
			assert(dydx0 > 0);
			assert(dydx1 > 0);
			ObjFunData fd;
			fd.dx = dx;
			fd.y0 = y0;
			fd.y1 = y1;
			fd.dydx0 = dydx0;
			fd.dydx1 = dydx1;
			fd.lambda = lambda;
			std::vector<double> x(DIM, 0.0);
			
			if (dydx0 > dydx1) {
				x[0] = 1.0 / dx;
				x[1] = 0;				
			} else {
				x[0] = 1.0 / dx;
				x[1] = -x[0] * dx;
			}
			//x[0] = 1.0 / dx;
			//x[1] = -x[0] * dx / 2;
			//const double tmp = tanh(x[0] * dx + x[1]);
			//x[2] = (y1 - y0) / (tmp - tanh(x[1]));
			//x[2] = y1 - x[2] * tmp;
			double value;
			/*nlopt::opt opt_pre(nlopt::LN_COBYLA, DIM);
			opt_pre.set_ftol_rel(1e-6);
			opt_pre.set_xtol_rel(1e-6);
			opt_pre.set_min_objective(objective_function, &fd);
			try {
				opt_pre.optimize(x, value);
			} catch (std::exception& e) {
				std::cerr << "Pre-Solver failure: " << e.what() << " for parameters " << y0 << " " << y1 << " " << dydx0 << " " << dydx1 << " " << dx << std::endl;
				throw e;
			}
			std::cerr << "Pre-Solver solution norm: " << value << std::endl;*/
			nlopt::opt opt(nlopt::LD_LBFGS, DIM);

			// ax >= 0, bx unconstrained
			const static std::vector<double> lb({ 0.0, -std::numeric_limits<double>::infinity() });
			opt.set_min_objective(objective_function, &fd);						
			opt.set_lower_bounds(lb);
			//opt.set_ftol_abs(1e-14 * std::abs(median) + 1e-15);
			opt.set_ftol_rel(1e-10);
			opt.set_xtol_rel(1e-10);
			nlopt::result ret;
			try {
				ret = opt.optimize(x, value);
			} catch (std::exception& e) {
				LOG_WARN() << "InterpolatorImplTanh: Solver failure: " << e.what() << " for parameters " << y0 << " " << y1 << " " << dydx0 << " " << dydx1 << " " << dx << "\n" << "Last value: " << value;
				set_linear(y0, (y1 - y0) / dx, ax, bx, ay, by);
				return value;
			}
			LOG_DEBUG() << "InterpolatorImplTanh: Solver solution norm: " << value << " and return code: " << ret;
			ax = x[0];
			bx = x[1];
			//ay = x[2];
			//by = x[2];
			ay = (y1 - y0) / (tanh(ax * dx + bx) - tanh(bx));
			by = y0 - ay * tanh(bx);
			return value;
		} else {
			assert(dydx0 < 0);
			assert(dydx1 < 0);
			// reflection in Y
			const double norm = calibrate_tanh_node(lambda, dx, -y0, -y1, -dydx0, -dydx1, ax, bx, ay, by);
			ay *= -1;
			by *= -1;
			return norm;
		}
		//if (y1 > y0) {
		//	assert(dydx0 > 0);
		//	assert(dydx1 > 0);
		//	if (dydx0 > dydx1) {
		//		// solve this numerically...

		//	} else {
		//		// reflection in X
		//		// y(x) = ay * tanh(ax * x + bx) + by
		//		// y(-x) = ay * tanh((-ax) * (-x) + bx) + by
		//		calibrate_tanh_node(dx, y1, y0, -dydx1, -dydx0, ax, bx, ay, by);
		//		ax *= -1;
		//	}
		//} else {
		//	// reflection in Y
		//	// y(x) = ay * tanh(ax * x + bx) + by
		//	// -y(x) = (-ay) * tanh(ax * x + bx) + (-by)
		//	calibrate_tanh_node(dx, -y0, -y1, -dydx0, -dydx1, ax, bx, ay, by);
		//	ay *= -1;
		//	by *= -1;
		//}
	}
}
