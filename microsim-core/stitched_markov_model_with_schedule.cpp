/*
* (C) Averisera Ltd 2017
*/
#include "stitched_markov_model_with_schedule.hpp"
#include "core/discrete_distribution.hpp"
#include "core/log.hpp"

namespace averisera {
	namespace microsim {
		template <class S> StitchedMarkovModelWithSchedule<S>::StitchedMarkovModelWithSchedule(const model_type& base, Period period, Date start_date, const Date cache_end_date)
			: base_(base), period_(period), start_date_(start_date)
		{
			const size_t cache_size = static_cast<size_t>(calc_model_time(cache_end_date));
			base_.precalculate_state_distributions(cache_size);
		}

		//StitchedMarkovModelWithSchedule::StitchedMarkovModelWithSchedule(StitchedMarkovModelWithSchedule&& other)
		//	: base_(std::move(other.base_)), period_(other.period_), start_date_(other.start_date_)
		//	//, cache_end_date_(other.cache_end_date_) 
		//{
		//	other.start_date_ = Date::NAD;
		//	//other.cache_end_date_ = Date::NAD;
		//	TRACE() << "StitchedMarkovModelWithSchedule: moved";
		//}

		template <class S> StitchedMarkovModelWithSchedule<S>::StitchedMarkovModelWithSchedule(const StitchedMarkovModelWithSchedule<S>& other)
			: base_(other.base_), period_(other.period_), start_date_(other.start_date_) {
			LOG_TRACE() << "StitchedMarkovModelWithSchedule: copied";
		}

		template <class S> typename StitchedMarkovModelWithSchedule<S>::state_type StitchedMarkovModelWithSchedule<S>::draw_next_state(state_type k, Date current_date, double u) const {
			if (current_date < start_date()) {
				return k;
			} else {				
				const auto t = calc_model_time(current_date);
				return base_.draw_next_state(k, t, u);
			}
		}

		template <class S> std::pair<typename StitchedMarkovModelWithSchedule<S>::state_type, double> StitchedMarkovModelWithSchedule<S>::draw_next_state_and_percentile(state_type k, Date current_date, double u) const {
			if (current_date < start_date()) {
				return std::make_pair(k, u);
			} else {
				const auto t = calc_model_time(current_date);
				return base_.draw_next_state_and_percentile(k, t, u);
			}
		}

		template <class S> typename StitchedMarkovModelWithSchedule<S>::time_type StitchedMarkovModelWithSchedule<S>::calc_model_time(Date d) const {
			const auto dist = d - start_date();
			return std::max(0L, dist.days() / period().days());
		}

		template <class S> Eigen::VectorXd StitchedMarkovModelWithSchedule<S>::calc_state_distribution(const Date d) const {
			const auto t = calc_model_time(d);
			return base_.calc_state_distribution(t);
		}

		template <class S> typename StitchedMarkovModelWithSchedule<S>::state_type StitchedMarkovModelWithSchedule<S>::draw_future_state(const Date d, const double u) const {
			const auto t = calc_model_time(d);
			const Eigen::VectorXd cdfs(base_.calc_state_cdf(t));
			return static_cast<typename StitchedMarkovModel<S>::state_type>(DiscreteDistribution::draw_from_cdf(cdfs.data(), cdfs.data() + base_.dim(), u));
		}

		template class StitchedMarkovModelWithSchedule<uint8_t>;
		template class StitchedMarkovModelWithSchedule<uint16_t>;
		template class StitchedMarkovModelWithSchedule<uint32_t>;
	}
}
