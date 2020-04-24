/*
* (C) Averisera Ltd 2017
*/
#ifndef __AVERISERA_MS_STITCHED_MARKOV_MODEL_WITH_SCHEDULE_HPP
#define __AVERISERA_MS_STITCHED_MARKOV_MODEL_WITH_SCHEDULE_HPP

#include "stitched_markov_model.hpp"
#include "core/dates.hpp"
#include "core/period.hpp"

namespace averisera {
	namespace microsim {
		/*! StitchedMarkovModel with a simple schedule

		\tparam S State class (unsigned int type)
		*/
		template <class S = unsigned int> class StitchedMarkovModelWithSchedule {
		public:
			typedef StitchedMarkovModel<S> model_type;
			typedef typename model_type::state_type state_type;
			//typedef StitchedMarkovModel::time_type time_type;

			/*!
			\param base Base models
			\param period Period of the models
			\param start_date Date when the 1st model is initialised
			\param cache_end_date First date for which we do not cache the extrapolated state probabilities & CDF
			*/
			StitchedMarkovModelWithSchedule(const model_type& base, Period period, Date start_date, Date cache_end_date);

			//StitchedMarkovModelWithSchedule(StitchedMarkovModelWithSchedule&& other);

			StitchedMarkovModelWithSchedule(const StitchedMarkovModelWithSchedule& other);
			StitchedMarkovModelWithSchedule& operator=(const StitchedMarkovModelWithSchedule& other) = delete;

			state_type dim() const {
				return base_.dim();
			}

			size_t nbr_models() const {
				return base_.nbr_models();
			}

			Period period() const {
				return period_;
			}

			Date start_date() const {
				return start_date_;
			}

			state_type draw_initial_state(double u) const {
				return base_.draw_initial_state(u);
			}

			state_type draw_next_state(state_type k, Date current_date, double u) const;

			std::pair<state_type, double> draw_next_state_and_percentile(state_type k, Date current_date, double u) const;

			Eigen::VectorXd calc_state_distribution(Date d) const;

			/*! Draw state from distribution returned by calc_state_distribution(d) 
			*/
			state_type draw_future_state(Date d, double u) const;
		private:
			model_type base_;
			Period period_;
			Date start_date_;

			typedef typename model_type::time_type time_type;
			time_type calc_model_time(Date d) const;
		};
	}
}

#endif // __AVERISERA_MS_STITCHED_MARKOV_MODEL_WITH_SCHEDULE_HPP
