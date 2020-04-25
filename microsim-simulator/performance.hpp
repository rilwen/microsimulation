#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "core/running_statistics.hpp"

namespace averisera {
	namespace microsim {
		/** Measures performance of a piece of code.

		Time unit: second
		*/
		class Performance {
		public:
			Performance();

			/**
			@param total_time Total time of call in seconds 
			@param nbr_processed Number of processed elements 
			@throw std::domain_error If total_time < 0 or nbr_processed == 0
			*/
			void append_metrics(double total_time, size_t nbr_processed);

			size_t total_nbr_processed() const {
				return total_nbr_processed_;
			}

			/** Return total execution time added up */
			double total_time() const {
				return total_time_stats_.mean() * static_cast<double>(total_time_stats_.nbr_samples());
			}

			const RunningStatistics<double>& total_time_stats() const {
				return total_time_stats_;
			}

			const RunningStatistics<double>& time_per_element_stats() const {
				return time_per_element_stats_;
			}

			const RunningStatistics<double>& nbr_processed_stats() const {
				return nbr_processed_stats_;
			}

			/** Execute functor, measure execution time and append metrics
			@tparam F Functor class with operator()()
			@param functor Functor object
			@param nbr_processed Number of processed elements
			@throws Whatever functor throws (if any) or std::domain_error if nbr_processed == 0
			*/
			template <class F> void measure_metrics(F functor, size_t nbr_processed) {
				const double total_time = measure_time(functor);
				append_metrics(total_time, nbr_processed);
			}
		private:
			RunningStatistics<double> total_time_stats_;
			RunningStatistics<double> time_per_element_stats_;
			RunningStatistics<double> nbr_processed_stats_;
			size_t total_nbr_processed_;

			template <class F> double measure_time(F functor) const {
				const clock_t time0 = std::clock();
				functor();
				const clock_t time1 = std::clock();
				const double total_time = static_cast<double>(time1 - time0) / CLOCKS_PER_SEC;
				return total_time;
			}
		};
	}
}
