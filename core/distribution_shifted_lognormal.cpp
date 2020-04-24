#include "distribution_shifted_lognormal.hpp"
#include "log.hpp"
#include "preconditions.hpp"
#include "nlopt_wrap.hpp"
#include "sacado_scalar.hpp"
#include "stl_utils.hpp"
#include <cassert>
#include <cmath>

namespace averisera {
    DistributionShiftedLognormal::DistributionShiftedLognormal(const double mu, const double sigma, const double shift)
        : _normal(mu, sigma, false), // do our own validation to get error messages with correct class name
		_shift(shift), _mean(shift + exp(mu + sigma * sigma / 2)),
          _variance(expm1(sigma * sigma) * exp(2 * mu + sigma * sigma)) {
		check_that(std::isfinite(mu), "DistributionShiftedLognormal: mu not finite");
		check_that(std::isfinite(sigma), "DistributionShiftedLognormal: sigma not finite");
		check_that(std::isfinite(shift), "DistributionShiftedLognormal: shift not finite");
		check_that(sigma >= 0, "DistributionShiftedLognormal: sigma negative");
    }

	DistributionShiftedLognormal::DistributionShiftedLognormal(const DistributionShiftedLognormal& other)
		: _normal(other._normal), _shift(other._shift), _mean(other._mean), _variance(other._variance) {}

    double DistributionShiftedLognormal::pdf(double y) const {
        assert(y >= _shift);
        if (y > _shift) {
            const double w = y - _shift;
            const double x = log(w);
            return _normal.pdf(x) / w;
        } else {
            return 0.;
        }
    }

    namespace {
        struct ObjFunData {
            ObjFunData(const std::vector<double>& n_sample)
                : sample(n_sample), N(n_sample.size()), n1(N/3), n2(N - n1) {}
            const std::vector<double>& sample;
            const size_t N;
            const size_t n1;
            const size_t n2;

            adouble pivotal(const adouble& shift) const {
                adouble s1(0.0);
                for (size_t i = 0; i < n1; ++i) {
                    s1 += log(sample[i] - shift);
                }
                s1 /= static_cast<double>(n1);
                adouble s2(0.0);
                for (size_t i = n1; i < n2; ++i) {
                    s2 += log(sample[i] - shift);
                }
                s2 /= static_cast<double>(N - 2 * n1);
                adouble s3(0.0);
                for (size_t i = n2; i < N; ++i) {
                    s3 += log(sample[i] - shift);
                }
                s3 /= static_cast<double>(n1);
                return (s2 - s1) / (s3 - s2);
            }
        };
    }


    static double objective_function(const std::vector<double>& x, std::vector<double>& grad, void* f_data) {
        const ObjFunData* data = static_cast<const ObjFunData*>(f_data);        
        assert(data);
        assert(x.size() == 1);
        //assert(grad.size() == 1);
        unsigned int idx = 0;
        const adouble shift(from_double<0>(1, idx, x[0]));
        assert(idx == 1);
        const std::pair<adouble, adouble> mu_var(DistributionShiftedLognormal::estimate_params_given_shift(data->sample.begin(), data->sample.end(), shift));
        //const adouble fitted_median(DistributionShiftedLognormal::median(mu_var.first, shift));
        //std::cout << "shift == " << shift.val() << ", fitted_median == " << fitted_median.val() << ", sample median == " << data->median << std::endl;
        //const adouble result(pow(fitted_median - data->median, 2));
        adouble result = pow( data->pivotal(shift) - 1.0, 2 );
        // for (double x : data->sample) {
        //     result += pow((x - mu_var.first) / mu_var.second / 2.0, 2);
        // }
        if (!grad.empty()) {
            grad[0] = result.dx(0);
        }
        return result.val();
    }

    DistributionShiftedLognormal DistributionShiftedLognormal::estimate(std::vector<double>& sample) {
        if (sample.size() < 10) {
            throw std::domain_error("DistributionShiftedLognormal: sample too small");
        }
        std::sort(sample.begin(), sample.end());
        const size_t n = sample.size();
        double median;
        if (n % 2 == 0) {
            median = 0.5 * sample[n / 2 - 1] + 0.5 * sample[n / 2];
        } else {
            median = sample[n / 2];
        }
        const double min_x = sample[0];
        const double bnd_eps = 1E-16 + 1E-14 * std::abs(min_x);
        //std::cout << "min_x == " << min_x << std::endl;
        ObjFunData fun_data(sample);
        nlopt::opt opt(nlopt::LD_LBFGS, 1);
        opt.set_min_objective(objective_function, &fun_data);
        opt.set_upper_bounds(std::vector<double>{min_x - bnd_eps});
        opt.set_ftol_abs(1e-14 * std::abs(median) + 1e-15);
        opt.set_ftol_rel(1e-8);
        opt.set_xtol_rel(1e-8);
        double value;
        std::vector<double> x(1);
        if (min_x < 0) {
            x[0] = std::min(min_x - 1, (1 + 1e-15) * min_x);
        } else {
            x[0] = std::min(min_x - 1, (1 - 1e-15) * min_x);
        }
        opt.optimize(x, value);
        return estimate_given_shift(sample.begin(), sample.end(), x[0]);
    }

	DistributionShiftedLognormal DistributionShiftedLognormal::fit_exactly_given_shift(const double p0, const double p1, 
		const double shift, const double x0, const double x1) {
		check_that(p0 > 0, "DistributionShiftedLognormal::fit_exactly_given_shift: p0 must be > 0");
		check_that(p1 > 0, "DistributionShiftedLognormal::fit_exactly_given_shift: p1 must be > 0");
		check_that(p0 + p1 < 1, "DistributionShiftedLognormal::fit_exactly_given_shift: p0 + p1 must be < 1");
		check_that(x1 > x0, "DistributionShiftedLognormal::fit_exactly_given_shift: x1 must be above x0");
		check_that(x0 > shift, "DistributionShiftedLognormal::fit_exactly_given_shift: x0 must be above the shift");
		const double y0 = NormalDistribution::normsinv(p0);
		const double y1 = NormalDistribution::normsinv(p0 + p1);
		assert(std::isfinite(y0));
		assert(std::isfinite(y1));
		assert(y1 > y0);
		const double a0 = std::log(x0 - shift);
		const double a1 = std::log(x1 - shift);
		const double sigma = (a1 - a0) / (y1 - y0);
		const double mu = (a0 + a1 - sigma * (y0 + y1)) / 2;
		return DistributionShiftedLognormal(mu, sigma, shift);
	}

	class DistributionShiftedLognormalKLDivergence {
	public:
		DistributionShiftedLognormalKLDivergence(const std::vector<double>& x, const std::vector<double>& p)
			: x_(x), p_(p), k_(x.size()) {
			assert(!x.empty());
			assert(p.size() == x.size());
			x_.push_back(std::numeric_limits<double>::infinity()); // makes code easier to write
		}

		double value_and_gradient(double mu, double sigma, double& over_mu, double& over_sigma) const {
			assert(sigma > 0);
			assert(std::isfinite(mu));
			double sum = 0;
			double sum_over_mu = 0;
			double partial_sum_over_sigma = 0;
			auto x_iter = x_.begin();
			auto p_iter = p_.begin();
			const double shift = x_.front();
			assert(std::isfinite(shift));
			double prev_cq= 0.0;
			double prev_pdf = 0.0;
			double prev_y_pdf = 0.0;
			++x_iter;
			for (; p_iter != p_.end(); ++p_iter, ++x_iter) {
				const double p = *p_iter;
				const double x = *x_iter;
				const double y = std::log(x - shift);
				const double z = (y - mu) / sigma;
				const double cq = NormalDistribution::normcdf(z);
				const double pdf = NormalDistribution::normpdf(z);
				const double y_pdf = std::isfinite(y) ? (y * pdf) : 0.0;
				assert(cq >= prev_cq);
				const double dq = cq - prev_cq;
				sum += p * std::log(p / dq);
				sum_over_mu += p * (pdf - prev_pdf) / dq;
				partial_sum_over_sigma += p * (y_pdf - prev_y_pdf) / dq;
				prev_cq = cq;
				prev_pdf = pdf;
				prev_y_pdf = y_pdf;
			}
			over_mu = sum_over_mu / sigma;
			over_sigma = (partial_sum_over_sigma / sigma - over_mu * mu) / sigma;
			//LOG_TRACE() << "y = " << sum << ", dy/dmu = " << over_mu << ", dy/dsigma = " << over_sigma;
			//LOG_TRACE() << sum;
			return sum;
		}

		double value(double mu, double sigma) const {
			assert(sigma > 0);
			assert(std::isfinite(mu));
			double sum = 0;
			auto x_iter = x_.begin();
			auto p_iter = p_.begin();
			const double shift = x_.front();
			assert(std::isfinite(shift));
			double prev_cq = 0.0;
			++x_iter;
			for (; p_iter != p_.end(); ++p_iter, ++x_iter) {
				const double p = *p_iter;
				const double x = *x_iter;
				const double y = std::log(x - shift);
				const double z = (y - mu) / sigma;
				const double cq = NormalDistribution::normcdf(z);
				//const double pdf = NormalDistribution::normpdf(z);
				//const double y_pdf = std::isfinite(y) ? (y * pdf) : 0.0;
				assert(cq >= prev_cq);
				const double dq = cq - prev_cq;
				sum += p * std::log(p / dq);
				prev_cq = cq;
			}
			//LOG_TRACE() << sum;
			return sum;			
		}
	private:
		std::vector<double> x_;
		const std::vector<double> p_;
		const size_t k_;
	};

	static double nlopt_f_distr_shifted_lognormal(const std::vector<double>& x, std::vector<double>& grad, void* param) {
		const DistributionShiftedLognormalKLDivergence* kldivptr = static_cast<const DistributionShiftedLognormalKLDivergence*>(param);
		assert(x.size() == 2);
		
		//LOG_TRACE() << "x = " << x;
		if (!grad.empty()) {
			assert(grad.size() == 2);
			return kldivptr->value_and_gradient(x[0], x[1], grad[0], grad[1]);
		} else {
			return kldivptr->value(x[0], x[1]);
		}
	}

	DistributionShiftedLognormal DistributionShiftedLognormal::estimate_given_shift(const std::vector<double>& x, const std::vector<double>& p) {
		DistributionShiftedLognormalKLDivergence kldiv(x, p);
		nlopt::opt opt(nlopt::LD_SLSQP, 2);
		opt.set_min_objective(nlopt_f_distr_shifted_lognormal, &kldiv);
		opt.set_lower_bounds(std::vector<double>{-std::numeric_limits<double>::infinity(), 1e-10});
		opt.set_ftol_abs(1e-10);
		opt.set_ftol_rel(1e-10);
		opt.set_xtol_rel(1e-10);
		double value;
		std::vector<double> solution(2);
		solution[0] = 0.0;
		solution[1] = 1.0;
		opt.optimize(solution, value);
		return DistributionShiftedLognormal(solution[0], solution[1], x.front());
	}
}
