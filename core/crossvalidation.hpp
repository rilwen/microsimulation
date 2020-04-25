/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_CROSSVALIDATION_H
#define __AVERISERA_CROSSVALIDATION_H

#include <Eigen/Core>
#include <algorithm>
#include <cassert>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iostream>
#include "math_utils.hpp"
#include "observed_discrete_data.hpp"
#include "preconditions.hpp"

namespace averisera {
	/** Cross-validation algorithms implemented as model-independent templates. */
	namespace CrossValidation {

		// Check if vector years is empty, and if it is, resize it to T and fill with 0, 1, ..., T - 1.
		// years: In/out vector
		// T: desired size
		void fix_years(std::vector<double>& years, size_t T);

		namespace {
			template <class M, class F, class I1, class I2> double error_on_omitted_cross_sectional(const ObservedDiscreteData& data0, M& model, F& divergence, I1 omit_idx_begin, I1 omit_idx_end, I2 error_idx_begin) {
				const size_t L = static_cast<size_t>(std::distance(omit_idx_begin, omit_idx_end)); // nbr omitted indices
				const size_t T0 = static_cast<size_t>(data0.nbr_surveys.size());
				assert(data0.times.size() == T0);
				const ObservedDiscreteData::lcidx_t dim = MathUtils::safe_cast<ObservedDiscreteData::lcidx_t>(data0.probs.rows());
				std::vector<size_t> srt_omt_idx(L);
				std::copy(omit_idx_begin, omit_idx_end, srt_omt_idx.begin());
				std::sort(srt_omt_idx.begin(), srt_omt_idx.end());
				std::vector<double> extrap_times(L);
				const size_t T = T0 - L;
				ObservedDiscreteData data(dim, T);

				// prepare input data
				size_t i0 = 0;
				size_t j = 0;
				for (size_t l = 0; l < L; ++l) {
					const size_t i1 = srt_omt_idx[l];
					extrap_times[l] = data0.times[i1];
					for (size_t i = i0; i < i1; ++i) {
						data.nbr_surveys[j] = data0.nbr_surveys[i];
						data.times[j] = data0.times[i];
						data.probs.col(j) = data0.probs.col(i);
						++j;
					}
					i0 = i1 + 1;
				}
				for (size_t i = i0; i < T0; ++i) {
					data.nbr_surveys[j] = data0.nbr_surveys[i];
					data.times[j] = data0.times[i];
					data.probs.col(j) = data0.probs.col(i);
					++j;
				}
				assert(j == T);

				Eigen::MatrixXd extrap_probs(dim, L);
				/*const double fit_norm = */model(data, extrap_times, extrap_probs);
				double err = 0;
				for (size_t i = 0; i < L; ++i) {
					const size_t k = srt_omt_idx[i];
					const double err_k = divergence(data0.nbr_surveys[k], data0.probs.col(k), extrap_probs.col(i));
					if (!std::isfinite(err_k)) {
						std::stringstream ss;
						ss << "Abnormal cross-sectional cross-validation error: " << err_k << ", P == " << data0.probs.col(k) << ", Q = " << extrap_probs.col(i) << std::endl;
						throw std::runtime_error(ss.str());
					}
					/*if (err_j > 10000) {
						std::cout << model.name() << ": omitted[" << i << " -> " << k << "] == " << err_j << "\n";
						std::cout << "fit_norm == " << fit_norm << "\n";
						std::cout << "data0.nbr_surveys[k] == " << data0.nbr_surveys[k] << "\n";
						std::cout << "data0.probs.col(k) == " << data0.probs.col(k) << "\n";
						std::cout << "extrap_probs.col(i) == " << extrap_probs.col(i) << std::endl;
					}*/
					err += err_k;
					*error_idx_begin = err_k;
					++error_idx_begin;
				}
				return err;
			}

			template <class V1, class V2> static void vec_to_vec(const V1& src, V2& dest) {
				const size_t len = src.size();
				dest.resize(len);
				std::copy(src.begin(), src.end(), dest.begin());
			}

			// Calculation method for longitudinal data
			template <class M, class I1> double minus_likelihood_of_omitted_longitudinal(const ObservedDiscreteData& data0, M& model, I1 omit_idx_begin, I1 omit_idx_end) {
				const size_t L = static_cast<size_t>(std::distance(omit_idx_begin, omit_idx_end)); // nbr omitted indices
				const size_t N0 = static_cast<size_t>(data0.ltimes.size());
				assert(data0.ltrajs.size() == N0);
				std::vector<size_t> srt_omt_idx(L);
				std::copy(omit_idx_begin, omit_idx_end, srt_omt_idx.begin());
				std::sort(srt_omt_idx.begin(), srt_omt_idx.end());
				const size_t N = N0 - L;

				ObservedDiscreteData calibration_data;
				ObservedDiscreteData omitted_data;
				typedef ObservedDiscreteData::lcidx_t lcidx_t;
				{ // copy not-omitted trajectories
					std::vector<std::vector<lcidx_t>> trajs(N);
					std::vector<std::vector<double>> times(N);
					std::vector<std::vector<lcidx_t>> omit_trajs(L);
					std::vector<std::vector<double>> omit_times(L);
					// prepare input data
					size_t j = 0;
					size_t k = 0;
					for (size_t i = 0; i < N0; ++i) {
						if (j == L || i != srt_omt_idx[j]) {
							vec_to_vec(data0.ltimes[i], times[k]);
							vec_to_vec(data0.ltrajs[i], trajs[k]);
							++k;
						} else {
							vec_to_vec(data0.ltimes[i], omit_times[j]);
							vec_to_vec(data0.ltrajs[i], omit_trajs[j]);
							++j;
						}
					}
					assert(k == N);
					assert(j == L);
					calibration_data = ObservedDiscreteData(times, trajs);
					omitted_data = ObservedDiscreteData(omit_times, omit_trajs);
				}

				const double t_cal = ObservedDiscreteData::first_time(calibration_data);
				const double t_omt = ObservedDiscreteData::first_time(omitted_data);
				if (t_cal != t_omt) {
					throw std::runtime_error("Different start times in calibration and test data sets");
				}

				// Return minus log-likelihood of omitted data
				return model(calibration_data, omitted_data).second;
			}
		}

		// Perform 1-out-of-N crossvalidation and return a vector of errors (calculated as a divergence of extrapolated probability distribution from the omitted one) for each year. Handles cross-sectional data only.
		//
		// data: Observed data (unchanged at exit).
		// model: a functor which performs the extrapolation through operator()(data, extrap_years, extrap_probs), where:
		//		1. data is the observed data object
		//		2. extrap_years is the vector of extrapolation years
		//		3. extrap_probs is a matrix with as many columns as extrap_years, to which the model should write the extrapolated distributions
		// divergence: a functor which calculates the weighted prob. distribution divergence through operator() (year_index, nbr_surveys_this_year, omitted_distribution, extrapolated_distribution).
		// leave_first: Do not remove the first time point - useful for models with memory which cannot extrapolate backward in time properly
		// Return: vector of errors
		template <class M, class F> std::vector<double> cross_validation(const ObservedDiscreteData& data, M& model, F& divergence, bool leave_first) {
			const size_t T = static_cast<size_t>(data.nbr_surveys.size());
			assert(data.times.size() == T);
			const size_t first_omitted = leave_first ? 1 : 0;
			std::vector<double> errorz(T - first_omitted);
			for (size_t omitted_t = first_omitted; omitted_t < T; ++omitted_t) {
				double error;
				error_on_omitted_cross_sectional(data, model, divergence, &omitted_t, (&omitted_t) + 1, &error);
				errorz[omitted_t - first_omitted] = error;
			}
			return errorz;
		}

		// k-fold cross-validation of longitudinal data. Splits trajectories into randomly chosen k subgroups. Measures error as minus log-likelihood.
		template <class M, class URNG> std::vector<double> cross_validation_kfold_longitudinal(const ObservedDiscreteData& data, const size_t k, M& model, URNG&& urng) {
			const size_t T = data.ltimes.size();
			std::vector<size_t> indices(T);
			std::iota(indices.begin(), indices.end(), 0);
			std::shuffle(indices.begin(), indices.end(), urng);
			std::vector<double> errorz(k);
			const size_t n_omit = indices.size();
			const size_t L = static_cast<size_t >(round(static_cast<double>(n_omit) / static_cast<double>(k)));
			assert(L > 0);
			for (size_t omitted = 0; omitted < k; ++omitted) {
				const auto omitted_init = L*omitted;
				const auto omitted_end = (omitted < k - 1) ? (omitted_init + L) : n_omit;
				const double total_error = minus_likelihood_of_omitted_longitudinal(data, model, indices.begin() + omitted_init, indices.begin() + omitted_end);
				errorz[omitted] = total_error;
			}
			return errorz;
		}

		// Time series cross-validation of longitudinal data. Calibrate the model to period [0, T)] and return the difference of the log-likelihood of calibration data and calibration data + next year.
		template <class M> double cross_validation_tseries_longitudinal(const ObservedDiscreteData& full_data, M& model) {
			if (full_data.nbr_surveys.size() > 0) {
				throw std::domain_error("Cross-sectional data not allowed");
			}
			const double t0 = ObservedDiscreteData::first_time(full_data);
			const double t1 = ObservedDiscreteData::last_time(full_data);
			const auto T = static_cast<size_t>(t1 - t0 + 1);
			if (T < 3) {
				throw std::domain_error("Too few samples");
			}

			const auto N = static_cast<size_t>(full_data.ltimes.size());
			assert(N);
			typedef ObservedDiscreteData::lcidx_t lcidx_t;
			std::vector<std::vector<double>> times(N);
			std::vector<std::vector<lcidx_t>> trajs(N);
			for (size_t k = 0; k < N; ++k) {
				vec_to_vec(full_data.ltimes[k], times[k]);
				vec_to_vec(full_data.ltrajs[k], trajs[k]);
			}

			double sum_err = 0;
			// assume time increment is integer
			for (auto t = T - 1; t >= 2; --t) {
				// test on data set containing time t
				ObservedDiscreteData test_data(times, trajs); // constructor copies data
				// remove points on or after t
				for (size_t k = 0; k < N; ++k) {
					// assume times are sorted
					std::vector<double>& trow = times[k];
					if (!trow.empty()) {
						assert(trow.back() <= t0 + static_cast<double>(t)); // later times were removed in previous iterations of loop over t
						if (trow.back() == t0 + static_cast<double>(t)) {
							assert(trow.size() == trajs[k].size());
							trow.pop_back();
							trajs[k].pop_back();
						}
					}
				}
				ObservedDiscreteData calibration_data(times, trajs);
				const auto lls = model(calibration_data, test_data); // (-log_likelihood(calibration_data), -log_likelihood(test_data))
				const double error = lls.second - lls.first;
				sum_err += error;
			}
			return sum_err;
		}

		// Perform k-fold crossvalidation and return a vector of errors (calculated as a divergence of extrapolated probability distribution from the omitted one) for each year. Handles cross-sectional data only.
		// Use preselected subsets.
		//
		// orig_data: Observed data
		// indices: Vector of permutated indices 0, ..., T - 1. It is split into parts of size T / k to generate crossvalidation subsets.
		// model: a functor which performs the extrapolation through operator()(data, extrap_years, extrap_probs), where:
		//		1. data is the observed data object
		//		2. extrap_years is the vector of extrapolation years
		//		3. extrap_probs is a matrix with as many columns as extrap_years, to which the model should write the extrapolated distributions
		// divergence: a functor which calculates the weighted prob. distribution divergence through operator() (year_index, nbr_surveys_this_year, omitted_distribution, extrapolated_distribution).
		// Return: errors
		template <class M, class F> std::vector<double> cross_validation_kfold(const ObservedDiscreteData& data, const std::vector<size_t>& indices, const size_t k, M& model, F divergence) {
			const size_t T = data.nbr_surveys.size();
			if (T < k) {
				throw std::domain_error("Too few samples");
			}
			check_equals(T, data.times.size());
			std::vector<double> errorz(k);
			const size_t n_omit = indices.size();
			const size_t L = static_cast<size_t >(round(static_cast<double>(n_omit) / static_cast<double>(k)));
			std::vector<double> err_per_t(L);
			assert(L > 0);
			for (size_t omitted = 0; omitted < k; ++omitted) {
				const auto omitted_init = L*omitted;
				const auto omitted_end = (omitted < k - 1) ? (omitted_init + L) : n_omit;
				err_per_t.resize(omitted_end - omitted_init);
				const double total_error = error_on_omitted_cross_sectional(data, model, divergence, indices.begin() + omitted_init, indices.begin() + omitted_end, err_per_t.begin());
				errorz[omitted] = total_error;
			}
			return errorz;
		}

		// Perform k-fold crossvalidation and return a vector of errors (calculated as a divergence of extrapolated probability distribution from the omitted one) for each year. Handles cross-sectional data only.
		// Choose the subsets randomly.
		//
		// data: Observed data
		// model: a functor which performs the extrapolation through operator()(data, extrap_years, extrap_probs), where:
		//		1. data is the observed data object
		//		2. extrap_years is the vector of extrapolation years
		//		3. extrap_probs is a matrix with as many columns as extrap_years, to which the model should write the extrapolated distributions
		// divergence: a functor which calculates the weighted prob. distribution divergence through operator() (year_index, nbr_surveys_this_year, omitted_distribution, extrapolated_distribution).
		// leave_first: Do not remove the first time point - useful for models with memory which cannot extrapolate backward in time properly
		// Return: pair (errors, indices)
		template <class M, class F, class URNG> std::pair<std::vector<double>, std::vector<size_t>> cross_validation_kfold(const ObservedDiscreteData& data, const size_t k, M& model, F divergence, URNG&& urng, bool leave_first) {
			const size_t offs = leave_first ? 1 : 0;
			const size_t T = data.nbr_surveys.size();
			std::vector<size_t> indices(T - offs);
			std::iota(indices.begin(), indices.end(), offs);
			std::shuffle(indices.begin(), indices.end(), urng);
			return std::make_pair(cross_validation_kfold(data, indices, k, model, divergence), indices);
		}

		// See http://robjhyndman.com/hyndsight/crossvalidation/, "Cross validation for time series"
		template <class M, class F> double cross_validation_tseries(const ObservedDiscreteData& data, M& model, F divergence) {
			const size_t T = data.nbr_surveys.size();
			if (T < 3) {
				throw std::domain_error("Too few samples");
			}
			check_equals(T, data.times.size());
			std::vector<double> errorz(T);
			std::vector<size_t> indices(T);
			std::iota(indices.begin(), indices.end(), 0);
			double sum_err = 0;
			for (size_t k = 2; k < T; ++k) {
				const auto omitted_begin = indices.begin() + k;
				error_on_omitted_cross_sectional(data, model, divergence, omitted_begin, omitted_begin + 1, errorz.begin());
				sum_err += errorz[0];
			}
			return sum_err;
		}
	}
}

#endif
