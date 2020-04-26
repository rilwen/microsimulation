// (C) Averisera Ltd 2014-2020
#include "operator_enforcer.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include "core/distribution.hpp"
#include "core/statistics.hpp"
#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../mutable_context.hpp"
#include "../person.hpp"

namespace averisera {
    namespace microsim {
        template <class T> OperatorEnforcer<T>::OperatorEnforcer(const std::string& variable, std::shared_ptr<const Predicate<T>> predicate,
                                                                 const std::vector<std::shared_ptr<const Distribution>>& distributions,
                                                                 HistoryFactory::factory_t history_factory,
                                                                 std::unique_ptr<Schedule>&& schedule)
            : Operator<T>(true, FeatureUser<Feature>::feature_set_t({variable})), hist_gen_(variable, history_factory, predicate),
            _variable(variable), _predicate(predicate), _distributions(distributions) {
            if (!predicate) {
                throw std::domain_error("OperatorEnforcer: null predicate");
            }
            if (std::any_of(distributions.begin(), distributions.end(), [](const std::shared_ptr<const Distribution>& distr) { return !distr; })) {
                throw std::domain_error("OperatorEnforcer: one or more distributions is null");
            }
            if (variable.empty()) {
                throw std::domain_error("OperatorEnforcer: empty variable name");
            }
            if (schedule && schedule->nbr_dates() > _distributions.size()) {
                throw std::out_of_range("OperatorEnforcer: not enough distributions for custom schedule");
            }
            _schedule = std::move(schedule);
        }
        
        template <class T> void OperatorEnforcer<T>::apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const {
            if (!_schedule) {                
                // custom schedule would have been checked in constructor
                if (contexts.immutable_ctx().schedule().nbr_dates() > _distributions.size()) {
                    throw std::out_of_range("OperatorEnforcer: not enough distributions");
                }
            } else {
                assert(contexts.immutable_ctx().schedule().contains(*_schedule));
            }
            
            const Date asof = contexts.asof();
            const Schedule::index_t idx = _schedule ? _schedule->index(asof) : contexts.asof_idx();
            const Distribution& distr = *(_distributions[idx]);
            const size_t n = selected.size();
            std::vector<double> v(n);
            std::vector<History*> histories(n);
            for (size_t i = 0; i < n; ++i) {      
                T& actor = *(selected[i]);
                History& history = actor.history(contexts.immutable_ctx(), _variable);
                histories[i] = &history; // histories are owned by the actor so they're not going away during the function call
                if (!history.empty()) {
                    // correct a value
                    v[i] = history.last_as_double(asof);
                } else {
                    // Draw fresh value
                    v[i] = distr.icdf(contexts.mutable_ctx().rng().next_uniform());
                }
            }
            Statistics::percentiles_inplace(v.begin(), v.end());
            for (size_t i = 0; i < n; ++i) {
                const double x = distr.icdf(v[i]);
                histories[i]->append_or_correct(asof, x);
            }
        }

        template class OperatorEnforcer<Actor>;
        template class OperatorEnforcer<Person>;
    }
}
