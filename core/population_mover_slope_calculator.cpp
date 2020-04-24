#include "log.hpp"
#include "nlopt_wrap.hpp"
#include "population_mover_slope_calculator.hpp"
#include "preconditions.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>

namespace averisera {
	PopulationMoverSlopeCalculator::PopulationMoverSlopeCalculator(double tolerance)
		: tolerance_(tolerance) {
		check_that(tolerance > 0.0);
	}

    // maximise the difference a[i] - distr[i] to enforce maximum gradient over rank
	double objective_function(const std::vector<double>& x, std::vector<double>& grad, void* f_data) {
		Eigen::Ref<const Eigen::VectorXd>* distr = static_cast<Eigen::Ref<const Eigen::VectorXd>*>(f_data);
		const size_t N = distr->size();
		assert(x.size() == N);
		double sum = 0;
		for (size_t i = 0; i < N; ++i) {
			const double d = x[i] - (*distr)[i];
			sum += 0.5 * d * d;
			if (!grad.empty()) {
				grad[i] = d;
			}
		}
		return sum;
	}

	double constraint_function(const std::vector<double>& x, std::vector<double>& grad, void* f_data) {
		if (!grad.empty()) {
			std::fill(grad.begin(), grad.end(), 1.0);
		}
		return std::accumulate(x.begin(), x.end(), 0.0) - 1.0;
	}

	void PopulationMoverSlopeCalculator::calculate(Eigen::Ref<const Eigen::VectorXd> distr, const size_t from_idx, std::vector<double>& a, std::vector<double>& b) const {
		const size_t N = distr.size();
		check_that(from_idx < N);
		a.resize(N);
		b.resize(N);

		// inequality constraints
		std::vector<double> lb(N);
		std::vector<double> ub(N);
		for (size_t i = 0; i < N; ++i) {
			if (i != from_idx) {
				const double p = distr[i];
				double l = std::max(0., 2 * p - 1);
				double u = std::min(1., 2 * p);
				if (i < from_idx) {
					// probability of jumping from from_idx to i < from_idx should decrease with rank,
                    // so b < 0 and a > p
					l = std::max(l, p);
				} else {
					// probability of jumping from from_idx to i > from_idx should increase with rank,
                    // so b > 0 and a < p
                    assert(i > from_idx);
					// else a < p
					u = std::min(u, p);
				}
				if (l > u) { // numerical inaccuracy pushes l above u
					// no non-trivial solution
					l = u = p;
				}
                assert((l < u) || ((l == p) && (u == p)));
				lb[i] = l;
				ub[i] = u;
			} else {
				lb[i] = ub[i] = distr[i];
			}
			LOG_DEBUG() << "Bounds[" << i << "]==(" << lb[i] << ", " << ub[i] << ")";
			a[i] = lb[i] + 0.5 * (ub[i] - lb[i]); // init. guess
		}

		const auto n = static_cast<unsigned int>(N);
		nlopt::opt opt(nlopt::LD_SLSQP, n);
		opt.set_vector_storage(std::max(10u, 2 * n));
		opt.set_max_objective(objective_function, &distr);
		opt.add_equality_constraint(constraint_function, nullptr, 1e-14);
		opt.set_lower_bounds(lb);
		opt.set_upper_bounds(ub);
		opt.set_ftol_rel(tolerance_);
		opt.set_xtol_rel(tolerance_);
		opt.set_maxeval(1000);

		double value;
		run_nlopt("PopulationMoverSlopeCalculator", opt, a, value);
        LOG_DEBUG() << "Value==" << value;

		calc_b(distr, a, b);
	}

	void PopulationMoverSlopeCalculator::calc_b(Eigen::Ref<const Eigen::VectorXd> distr, const std::vector<double>& a, std::vector<double>& b) const {
		const size_t N = distr.size();
		for (size_t i = 0; i < N; ++i) {
			b[i] = 2 * (distr[i] - a[i]);
		}
	}
}
