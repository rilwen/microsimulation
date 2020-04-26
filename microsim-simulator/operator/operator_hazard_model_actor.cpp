// (C) Averisera Ltd 2014-2020
#include "operator_hazard_model_actor.hpp"
#include "../actor.hpp"
#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../history_registry.hpp"
#include "../person.hpp"
#include "core/math_utils.hpp"
#include <cassert>

namespace averisera {
    namespace microsim {
        template <class T> OperatorHazardModelActor<T>::OperatorHazardModelActor(const Feature& provided_feature, const HazardModel& hazard_model
            , const std::vector<std::shared_ptr<const RelativeRisk<T>>>& relative_risks
            , std::shared_ptr<const Predicate<T>> predicate, const std::string& state_variable
            , HistoryFactory::factory_t history_factory, std::unique_ptr<Schedule>&& schedule)
            : OperatorHazardModel<T>(provided_feature, hazard_model, relative_risks, predicate, std::move(schedule)),
            hist_gen_(state_variable, history_factory, predicate),
            _state_variable(state_variable)
        {}

        template <class T> unsigned int OperatorHazardModelActor<T>::current_state(const T& obj, const Contexts& ctx, const std::string& state_variable) {
            const auto state_variable_idx = obj.get_variable_index(ctx, state_variable);
            const History& state_history = obj.history(state_variable_idx);
            return static_cast<unsigned int>(state_history.last_as_int());
        }

        template <class T> void OperatorHazardModelActor<T>::set_next_state(T& obj, Date date, unsigned int state, const Contexts& ctx, const std::string& state_variable) {
            const auto state_variable_idx = obj.get_variable_index(ctx, state_variable);
            History& state_history = obj.history(state_variable_idx);
            state_history.append(date, MathUtils::safe_cast<History::int_t>(state));
        }

        template <class T> unsigned int OperatorHazardModelActor<T>::current_state(const T& obj, const Contexts& ctx) const {
            return current_state(obj, ctx, _state_variable);
        }

        template <class T> void OperatorHazardModelActor<T>::set_next_state(T& obj, Date date, unsigned int state, const Contexts& ctx) const {
            set_next_state(obj, date, state, ctx, _state_variable);
        }        
        
        template class OperatorHazardModelActor<Actor>;
        template class OperatorHazardModelActor<Person>;
    }
}
