/*
 * (C) Averisera Ltd 2014
 */
#include "statistics.hpp"
#include "observed_discrete_data.hpp"
#include "running_statistics.hpp"
#include "eigen.hpp"
#include "log.hpp"
#include "multi_index.hpp"
#include "preconditions.hpp"
#include <cmath>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    namespace Statistics {
        static const double log_2pi = log(2 * 3.141592653589793);
        
        double bic(double log_likelihood, unsigned int number_parameters, unsigned int number_observations) {
            return -2 * log_likelihood + number_parameters * (log(number_observations) - log_2pi);
        }
        
        double aic(double log_likelihood, unsigned int number_parameters, unsigned int number_observations) {
            return 2 * (-log_likelihood + number_parameters);
        }
        
        double aic_corr(double log_likelihood, unsigned int number_parameters, unsigned int number_observations) {
            return 2 * (-log_likelihood + number_parameters + number_parameters*(number_parameters + 1.0) / (number_observations - number_parameters - 1.0));
        }
        
		std::vector<double> autocorrelations(const ObservedDiscreteData& data) {
            std::vector<RunningStatistics<double>> stats(1);
            // variance
            for (auto it = data.ltrajs.flat_begin(); it != data.ltrajs.flat_end(); ++it) {
                stats.front().add(*it);
            }
            const double var = stats.front().variance();
            // covariances
            const auto rend = data.ltrajs.row_end();
            auto time_rit = data.ltimes.row_begin();
            for (auto rit = data.ltrajs.row_begin(); rit != rend; ++rit) {
                const auto row = *rit;
                const auto time_row = *time_rit;
                const size_t n = row.size();
                assert(n == time_row.size());
                for (size_t i = 1; i < n; ++i) {
                    const double t_i = time_row[i];
                    const auto k_i = row[i];
                    for (size_t j = 0; j < i; ++j) {
                        const double t_j = time_row[j];
                        if (t_j >= t_i) {
                            std::stringstream ss;
                            ss << "Time " << t_j << " should fall before " << t_i;
                            throw std::runtime_error(ss.str());
                        }
                        const size_t lag = static_cast<size_t>(t_i - t_j);
                        if (stats.size() <= lag) {
                            stats.resize(lag + 1);
                        }
                        stats[lag].add(k_i * row[j]);
                    }
                }
                ++time_rit;
            }
            assert(time_rit == data.ltimes.row_end());
            const size_t sn = stats.size();
            std::vector<double> ac(sn);
            std::transform(stats.begin(), stats.end(), ac.begin(), [var](const RunningStatistics<double>& st){return st.variance() / var; });
            return ac;
        }
        
        template <class V> static unsigned int calc_state_index(const V& trajectory, const unsigned int t, const unsigned int dim, const unsigned int memory) {
            unsigned int idx = 0;
            for (unsigned int q = t - memory; q <= t; ++q) {
                idx = idx * dim + trajectory[q];
            }
            return idx;
        }
        
        

		Eigen::VectorXd standard_deviations_delta(const Eigen::MatrixXd& covariance, const Eigen::MatrixXd& deltas, const double negative_variance_tolerance) {
            check_equals(covariance.rows(), covariance.cols());
			check_greater_or_equal(negative_variance_tolerance, 0, "Negative variance tolerance must be >= 0");
            const size_t dim = static_cast<size_t>(covariance.rows());
            check_equals(dim, static_cast<size_t>(deltas.rows()));
            const size_t N = static_cast<size_t>(deltas.cols());
            Eigen::VectorXd sigmas(N);
            for (size_t i = 0; i < N; ++i) {
                // out[i] = out0[i] + sum_k delta(k, i) * X(k)
                // Var(out[i]) = sum_k sum_l delta(k, i) * delta(l, i) * covariance(k, l)
                const double tmp = deltas.col(i).dot(covariance * deltas.col(i));
				if (tmp < -negative_variance_tolerance) {
					throw std::runtime_error("Provided parameter covariance leads to negative output variance in excess of provided tolerance");
				}
                const double var = std::max(tmp, 0.);
                sigmas[i] = sqrt(var);
            }
            return sigmas;
        }
        
        void estimate_covariance_matrix(Eigen::Ref<const Eigen::MatrixXd> data, DataCheckLevel check_level, Eigen::MatrixXd& cov) {
            const Eigen::VectorXd means(EigenUtils::accurate_mean_colwise(data, check_level, false));
            if (data.rows() < 2) {
                throw std::domain_error("Statistics: Not enough rows in data for covariance matrix estimation");
            }
            const size_t row_cnt = estimate_covariance_matrix(data, means, check_level, cov);
			if (row_cnt < 2) {
				throw std::runtime_error("Statistics: too many rows excluded by data checks to estimate covariance");
			}
            const double row_fcnt = static_cast<double>(row_cnt);
            cov *= row_fcnt / (row_fcnt - 1.0); // get unbiased estimator
        }
        
        size_t estimate_covariance_matrix(Eigen::Ref<const Eigen::MatrixXd> data, Eigen::Ref<const Eigen::VectorXd> means, DataCheckLevel check_level, Eigen::MatrixXd& cov) {
            const size_t dim = static_cast<size_t>(data.cols());
            cov.setZero(dim, dim);
            const size_t nrows = static_cast<size_t>(data.rows());
            if (nrows < 1) {
                throw std::domain_error("Not enough rows in data for covariance matrix estimation");
            }
			size_t row_cnt = 0;
			for (size_t i = 0; i < nrows; ++i) {
                const auto row(data.row(i));
				if (!MathUtils::check_data(row, check_level)) {
					continue;
				}
				++row_cnt;
                for (size_t j1 = 0; j1 < dim; ++j1) {
                    auto cov_row(cov.row(j1));
                    const double x1 = row[j1] - means[j1];
                    for (size_t j2 = 0; j2 <= j1; ++j2) {
                        const double x2 = row[j2] - means[j2];
                        cov_row[j2] += (x1 * x2 - cov_row[j2]) / static_cast<double>(row_cnt);
                    }
                }
            }
            for (size_t j1 = 0; j1 < dim; ++j1) {
                auto cov_row(cov.row(j1));
                for (size_t j2 = 0; j2 < j1; ++j2) {
                    cov(j2, j1) = cov_row[j2];
                }
            }
			return row_cnt;
        }

        bool check_covariance_matrix(Eigen::Ref<const Eigen::MatrixXd> cov, std::string* message) {
            
            if (cov.cols() != cov.rows()) {
                if (message) {
                    *message = "Matrix is not square";
                }
                return false;
            }

            const size_t dim = static_cast<size_t>(cov.rows());
            for (size_t r = 0; r < dim; ++r) {
                const double var = cov(r, r);
                if (std::isnan(var)) {
                    if (message) {
                        *message = boost::str(boost::format("NaN variance at index %d") % r);
                    }
                    return false;
                }
                if (var < 0) {
                    if (message) {
                        *message = boost::str(boost::format("Negative variance at index %d: %g") % r % var);
                    }
                    return false;
                }
                for (size_t c = 0; c < r; ++c) {
                    const double cov_rc = cov(r, c);
                    if (std::isnan(cov_rc)) {
                        if (message) {
                            *message = boost::str(boost::format("NaN covariance at indices %d, %d") % r % c);
                        }
                        return false;
                    }
                    if (cov_rc != cov(c, r)) {
                        if (message) {
                            *message = boost::str(boost::format("Covariance is asymmetric at indices %d, %d: %g vs %g") % r % c % cov_rc % cov(c, r));
                        }
                        return false;
                    }
                    const double maxcov = sqrt(var * cov(c, c));
                    if (std::abs(cov_rc) > maxcov) {
                        if (message) {
                            *message = boost::str(boost::format("Correlation outside bounds at indices %d, %d: abs(%g) > %g") % r % c % cov_rc % maxcov);
                        }
                        return false;
                    }
                }
            }
            return true;
        }

        bool check_correlation_matrix(Eigen::Ref<const Eigen::MatrixXd> rho, std::string* message) {
            if (check_covariance_matrix(rho, message)) {
                const unsigned int dim = static_cast<unsigned int>(rho.rows());
                for (unsigned int i = 0; i < dim; ++i) {
                    if (rho(i, i) != 1) {
                        if (message) {
                            *message = boost::str(boost::format("Diagonal correlation is not 1 at index %d: %g") % i % rho(i, i));
                        }
                        return false;
                    }
                }
                return true;
            } else{
                return false;
            }
        }

		double sum_squared_differences_pairwise(const std::vector<double>& v) {
			const auto n = static_cast<double>(v.size());
			if (n > 1) {
				RunningStatistics<double> rs;
				std::for_each(v.begin(), v.end(), [&rs](double x) { rs.add(x); });
				return rs.variance() * (n - 1.0) * n;
			} else if (n == 1) {
				return 0.0;
			} else {
				return std::numeric_limits<double>::quiet_NaN();
			}
		}
    }
}
