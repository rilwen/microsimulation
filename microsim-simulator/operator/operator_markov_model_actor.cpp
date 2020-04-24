#include "../history.hpp"
#include "../history_factory.hpp"
#include "operator_markov_model_actor.hpp"
#include "../person.hpp"
#include "core/math_utils.hpp"
#include "core/preconditions.hpp"

namespace averisera {
    namespace microsim {
        template <class T> OperatorMarkovModelActor<T>::OperatorMarkovModelActor(const FeatureUser<Feature>::feature_set_t& required,
                                                                                 const std::string& variable,
                                                                                 MarkovModel&& markov_model,
                                                                                 std::shared_ptr<const Predicate<T>> pred,
                                                                                 bool initialize,
                                                                                 Array2D<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks_transitions,
                                                                                 std::vector<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks_initial_state,
                                                                                 std::unique_ptr<Schedule>&& schedule,
			HistoryFactory::factory_t history_factory)
        : OperatorMarkovModel<T>(required, variable, std::move(markov_model), pred, initialize, std::move(relative_risks_transitions), std::move(relative_risks_initial_state),
                                 std::move(schedule)),
            hist_gen_(variable, history_factory, pred) {
			check_not_null(history_factory, "OperatorMarkovModelActor: history factory cannot be null");
			check_that(!variable.empty(), "OperatorMarkovModelActor: variable name is empty");
        }

        template <class T> bool OperatorMarkovModelActor<T>::is_initialized(const T& obj, const Contexts& contexts) const {
            const auto state_variable_idx = obj.get_variable_index(contexts, this->variable());
            const ImmutableHistory& state_history = obj.history(state_variable_idx);
            return !state_history.empty();
        }
        
        template <class T> std::pair<Date, unsigned int> OperatorMarkovModelActor<T>::get_last_date_and_state(const T& obj, const Contexts& ctx) const {
            const auto state_variable_idx = obj.get_variable_index(ctx, this->variable());
            const ImmutableHistory& state_history = obj.history(state_variable_idx);
            return std::make_pair(state_history.last_date(), MathUtils::safe_cast<unsigned int>(state_history.last_as_int()));
        }
        
        template <class T> void OperatorMarkovModelActor<T>::set_next_state(T& obj, Date date, unsigned int state, const Contexts& ctx) const {
            const auto state_variable_idx = obj.get_variable_index(ctx, this->variable());
            History& state_history = obj.history(state_variable_idx);
			const auto value = MathUtils::safe_cast<History::int_t>(state);
			LOG_TRACE() << "OperatorMarkovModelActor: setting value " << value << " as of " << date;
            state_history.append(date, value);
        }

        template class OperatorMarkovModelActor<Actor>;
        template class OperatorMarkovModelActor<Person>;

    }
}
