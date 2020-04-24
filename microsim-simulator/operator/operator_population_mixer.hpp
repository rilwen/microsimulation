#pragma once
#include "../history_user_simple.hpp"
#include "../operator.hpp"
#include "core/population_mover.hpp"

namespace averisera {
	namespace microsim {
		/*! Uses Markov model to move entities between ranges of a continuous variable 
		\tparam T Derived from Actor */
		template <class T> class OperatorPopulationMixer: public Operator<T> {
		public:
			OperatorPopulationMixer(const std::string& variable, const Eigen::MatrixXd& pi, const std::vector<double>& ranges, std::shared_ptr<const Predicate<T>> predicate, std::unique_ptr<Schedule>&& schedule);

			/*! \throw std::logic_error If custom schedule does not fit in context schedule */
			void apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const override;

			const Predicate<T>& predicate() const override {
				return *predicate_;
			}

			bool active(Date date) const override {
				return Operator<T>::active(schedule_, date);
			}

			const typename HistoryUser<T>::use_reqvec_t& user_requirements() const override {
				return history_user_.user_requirements();
			}

			const std::string& name() const override {
				static const std::string str("PopulationMixer");
				return str;
			}
		private:
			HistoryUserSimple<T> history_user_;
			PopulationMover mover_;
			std::string variable_;
			std::shared_ptr<const Predicate<T>> predicate_;
			std::unique_ptr<Schedule> schedule_;
		};
	}
}
