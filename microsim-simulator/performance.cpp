/*
(C) Averisera Ltd 2017
*/
#include "performance.hpp"
#include "core/preconditions.hpp"

namespace averisera {
	namespace microsim {
		Performance::Performance()
		: total_nbr_processed_(0) {}

		void Performance::append_metrics(const double total_time, const size_t nbr_processed) {
			check_greater_or_equal(total_time, 0.0, "Performance::append_metrics: time");
			check_greater(nbr_processed, static_cast<size_t>(0), "Performance::append_metrics: nbr_processed");
			total_nbr_processed_ += nbr_processed;
			total_time_stats_.add(total_time);
			const double n = static_cast<double>(nbr_processed);
			nbr_processed_stats_.add(n);
			time_per_element_stats_.add(total_time / n);
		}
	}
}
