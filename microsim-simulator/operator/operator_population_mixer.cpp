#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../mutable_context.hpp"
#include "operator_population_mixer.hpp"
#include "../person.hpp"
#include "core/preconditions.hpp"
#include <cassert>

namespace averisera {
	namespace microsim {
		template <class T> OperatorPopulationMixer<T>::OperatorPopulationMixer(const std::string& variable, const Eigen::MatrixXd& pi, const std::vector<double>& ranges, std::shared_ptr<const Predicate<T>> predicate, std::unique_ptr<Schedule>&& schedule)
			: Operator<T>(false, FeatureUser<Feature>::feature_set_t({ variable })), 
			history_user_(variable),
			mover_(pi, ranges), variable_(variable), predicate_(predicate) {
			check_that(predicate != nullptr, "OperatorPopulationMixer: null predicate");
			check_that(!variable.empty(), "OperatorPopulationMixer: variable is empty");
			schedule_ = std::move(schedule);
		}

		template <class T> void OperatorPopulationMixer<T>::apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const {
			if (schedule_) {
				assert(contexts.immutable_ctx().schedule().contains(*schedule_));
			}
			std::vector<PopulationMover::Member> members;
			members.reserve(selected.size());
			const Date asof = contexts.asof();
			const Schedule& schedule = schedule_ ? *schedule_ : contexts.immutable_ctx().schedule();
			const Schedule::index_t date_idx = schedule_ ? schedule.index(asof) : contexts.asof_idx();			
			if (date_idx + 1 < schedule.nbr_dates()) {
				size_t member_idx = 0;
				for (const auto& ptr : selected) {
					assert(member_idx < selected.size());
					const ImmutableHistory& history = ptr->history(contexts.immutable_ctx(), variable_);
					const double x = history.last_as_double(asof);
					members.push_back({ member_idx, 0, x, 0 }); // TODO: choose the right distribution and range index
					++member_idx;
				}
				mover_.move_between_ranges(members, contexts.mutable_ctx().rng());
				const Date next_date = schedule.date(date_idx + 1);
				for (const auto& m : members) {
					assert(m.distribution_index == 0);
					History& history = selected[m.member_index]->history(contexts.immutable_ctx(), variable_);
					history.append(next_date, m.value);
				}
			}
		}

		template class OperatorPopulationMixer<Actor>;
		template class OperatorPopulationMixer<Person>;
	}
}
