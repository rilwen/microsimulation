#pragma once
#include "interpolator_impl_piecewise.hpp"
#include <vector>

namespace averisera {
	/** Interpolator used for strictly monotonic data 

	TODO: better calibration
	*/
	class InterpolatorImplTanh : public InterpolatorImplPiecewise {
	public:
		/** @throw std::domain_error If x.size() != y.size(), x.size() <= 1, y is not strictly monotonic or provided boundary conditions for dy/dx
		do not match the monotonicity of the data */
		InterpolatorImplTanh(std::vector<double>&& x, const std::vector<double>& y, double lambda = 1e-2);
		std::shared_ptr<InterpolatorImpl> clone() const override;
		InterpolatorImpl& operator+=(double x) override;
		InterpolatorImpl& operator-=(double x) override;
		InterpolatorImpl& operator*=(double x) override;
		InterpolatorImpl& operator/=(double x) override;
		using InterpolatorImplPiecewise::evaluate;
		// implementation - exposed for testing
		static double calibrate_tanh_node(double lambda, double dx, double y0, double y1, double dydx0, double dydx1, double& ax, double& bx, double& ay, double& by);

		double evaluate_derivative(double x) const;
		double evaluate_derivative(double x, size_t seg_idx) const;
	private:
		InterpolatorImplTanh(const InterpolatorImplTanh& other);
		double evaluate_impl(double x, size_t seg_idx) const override;
		void calibrate(const std::vector<double>& y);

		/** y(x) = ay * atanh(ax*(x - x[i]) + bx) + by */
		struct Node {
			double ay;
			double by;
			double ax;
			double bx;
		};

		std::vector<Node> nodes_;
		double lambda_;
	};
}
