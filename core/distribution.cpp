/*
  (C) Averisera Ltd 2014
  Author: Agnieszka Werpachowska
*/
#include "distribution.hpp"
#include "interpolator.hpp"
#include "preconditions.hpp"
#include "segment_search.hpp"
#include "statistics.hpp"
#include "stl_utils.hpp"
#include "adapt_1d.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <numeric>
#include <boost/format.hpp>

namespace averisera {
    size_t Distribution::idx(const std::vector<double>& xt, const double x) {
        if (x <= xt.front()) {
            return 0;
        } else if (x > xt.back()) {
            return xt.size();
        } else {
            const std::vector<double>::const_iterator it = std::lower_bound(xt.begin(), xt.end(), x);
            const size_t result = it - xt.begin();
            assert(result == 0 || x > xt[result - 1]);
            assert(result == xt.size() || x <= xt[result]);
            return result;
        }
    }

    double Distribution::range_prob(double x1, double x2) const {
        if (x2 < x1) {
            return 0.;
        } else {
            return cdf(x2) - cdf(x1);
        }
    }

    double Distribution::range_prob2(double x1, double x2) const {
        if (x2 < x1) {
            return 0.;
        } else {
            return cdf2(x2) - cdf2(x1);
        }
    }

    void Distribution::calculate_cumulative_proba(const std::vector<double>& p, std::vector<double>& cp, const double tol) {
        assert(p.size() == cp.size());
        assert(!p.empty());
        std::partial_sum(p.begin(), p.end(), cp.begin());
		if (tol >= 0 && std::fabs(cp.back() - 1.0) > tol) {
			throw std::domain_error(boost::str(boost::format("Distribution: sum of probabilities differs from 1: %g") % cp.back()));
		}
        cp.back() = 1.0; // Enforce sum == 1.0
    }

    void Distribution::draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const {
        if (x.size() != 1) {
            throw std::domain_error("Distribution: bad size");
        }
        x[0] = draw(rng);
    }

    void Distribution::draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const {
        if (x.size() != 1) {
            throw std::domain_error("Distribution: bad size");
        }
        x[0] = draw(rng);
    }

    void Distribution::marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const {
        if (x.size() != 1 || p.size() != 1) {
            throw std::domain_error("Distribution: bad size");
        }
        p[0] = cdf(x[0]);
    }

    void Distribution::marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const {
        if (x.size() != 1 || p.size() != 1) {
            throw std::domain_error("Distribution: bad size");
        }
        x[0] = icdf(p[0]);
    }

    void Distribution::adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const {
        if (sample.cols() != 1) {
            throw std::domain_error("Distribution: multiple columns");
        }
        auto col = sample.col(0); // assumes col-wise storage
        const unsigned int nr = static_cast<unsigned int>(sample.rows());
        Statistics::percentiles_inplace(col.data(), col.data() + nr);
        std::transform(col.data(), col.data() + nr, col.data(), [this](double p){ return icdf(p); });
    }

    // static double mean_integrand(const unsigned int* dim, const double* x, const unsigned int* npara, const double* params) {
    //     assert(*dim == 1);
    //     assert(*npara == 1);
    //     const Distribution* const distr = static_cast<const Distribution*>(AdaptIntegration::get_ptr(params[0]));
    //     return distr->icdf(x[0]);
    // }

    static const Adapt1D_15 _INTEGRATOR(100, 1E-8, false);

    double Distribution::mean() const {
        double p0, p1;
        calc_prob_bounds(p0, p1);
        return calc_mean_integral(p0, p1) / (p1 - p0);
    }

    double Distribution::variance(double mean) const {
        double p0, p1;
        calc_prob_bounds(p0, p1);
        return calc_variance_integral(p0, p1, mean) / (p1 - p0);
    }

    double Distribution::conditional_mean(const double a, const double b) const {
        if (b <= a) {
            throw std::domain_error("Distribution: b <= a");
        }
        double p0, p1;
        calc_prob_bounds(p0, p1, a, b);
        return calc_mean_integral(p0, p1) / (p1 - p0);
    }

    double Distribution::conditional_variance(const double conditional_mean, const double a, const double b) const {
        if (b <= a) {
            throw std::domain_error("Distribution: b <= a");
        }
        if (conditional_mean < a || conditional_mean >= b) {
            throw std::domain_error("Distribution: conditional_mean outside [a, b)");
        }
        double p0, p1;
        calc_prob_bounds(p0, p1, a, b);
        return calc_variance_integral(p0, p1, conditional_mean) / (p1 - p0);
    }

    void Distribution::calc_prob_bounds(double& p0, double& p1, double a, double b) const {
        if (a == -std::numeric_limits<double>::infinity()) {
            p0 = calc_prob_lower_bound();
        } else {
            p0 = cdf(a);
        }
        if (b == std::numeric_limits<double>::infinity()) {
            p1 = calc_prob_upper_bound();
        } else {
            p1 = cdf(b);
        }                    
    }

    double Distribution::calc_prob_bound_impl(const double bnd_if_icdf_finite, const double bnd_if_icdf_infinite) const {
        if (std::isfinite(icdf(bnd_if_icdf_finite))) {
            return bnd_if_icdf_finite;
        } else {
            return bnd_if_icdf_infinite;
        }
    }

    double Distribution::calc_mean_integral(double p0, double p1) const {
        return _INTEGRATOR.integrate([this](double p){ return icdf(p); }, p0, p1);
    }

    double Distribution::calc_variance_integral(double p0, double p1, double mean) const {
        return std::max(_INTEGRATOR.integrate([this, mean](double p){
                    const double x = icdf(p) - mean;
                    return x * x;
                }, p0, p1),
            0.0);
    }

	std::vector<double> Distribution::interpolate_continuous_cdf(const std::vector<double>& x, const std::vector<double>& cdf, const std::vector<double>& new_x) {
		check_equals(0.0, cdf.front());
		check_equals(1.0, cdf.back());
		return Interpolator::interpolate_y_linearly(x, cdf, new_x);
	}

	void Distribution::map_values_via_cdfs(const std::vector<double>& x, const std::vector<double>& cdf1, const std::vector<double>& cdf2, std::vector<double>& x1, std::vector<double>& x2) {
		const std::vector<double> cdf_ranges(StlUtils::merge_sorted_vectors(cdf1, cdf2));
		x1 = Interpolator::interpolate_y_linearly(cdf1, x, cdf_ranges);
		x2 = Interpolator::interpolate_y_linearly(cdf2, x, cdf_ranges);
	}

	void Distribution::validate(const std::vector<double>& x, const std::vector<double>& p) {
		const size_t n = p.size();
		if (n < 1) {
			throw std::domain_error("Distribution: at least 1 range needed");
		}
		if (x.size() != n + 1) {
			throw std::domain_error("Distribution: incorrect number of range bounds");
		}
		double sum = 0.;
		for (auto it = p.begin(); it != p.end(); ++it) {
			const double pp = *it;
			if (pp < 0 || pp > 1) {
				throw std::domain_error("Distribution: probability outside [0, 1]");
			}
			sum += pp;
		}
		if (std::abs(sum - 1.0) > 1E-8) {
			throw std::domain_error("Distribution: probabilities do not add up to 1");
		}
	}

	std::function<double(double)> Distribution::icdf_as_function(std::shared_ptr<const Distribution> distr) {
		check_that(distr != nullptr, "Distribution::icdf_as_function: null distribution");
		return std::function<double(double)>([distr](double p) {
			return distr->icdf(p);
		});
	}

	std::function<double(double)> Distribution::icdf_as_function(std::shared_ptr<const Distribution> distr, double upper_limit) {
		check_that(distr != nullptr, "Distribution::icdf_as_function: null distribution");
		return std::function<double(double)>([distr, upper_limit](double p) {
			return std::min(upper_limit, distr->icdf(p));
		});
	}
}
