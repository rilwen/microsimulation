// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_OPERATOR_MARKOV_MODEL_HPP
#define __AVERISERA_MICROSIM_OPERATOR_MARKOV_MODEL_HPP

#include "../contexts.hpp"
#include "../immutable_context.hpp"
#include "../operator_individual.hpp"
#include "../relative_risk.hpp"
#include "microsim-core/markov_model.hpp"
#include "core/array_2d.hpp"
#include "core/rng.hpp"
#include <cassert>
#include <limits>
#include <utility>

namespace averisera {
    namespace microsim {

        /** Operator which simulated the Markov process. */
        template <class T> class OperatorMarkovModel: public OperatorIndividual<T> {
        public:            
            /** Construct the operator as provider of feature = veriable name
              @param variable Variable name of process history
              @param markov_model Markov model to apply
              @param pred Operator predicate
              @param initialize Whether the operator should initialize an un-initialized object with initial state of the Markov process.
              @param relative_risks_transitions Array2D in which relative_risks_transitions[from][to] is the relative risk to be applied to the transition
              @param relative_risks_initial_state Vector of relative risk implementations for each state of Markov process used for initial state selection. Null element means no relative risk.
              @param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule.
              @throw std::domain_error If pred is null. If relative_risks_initial_state.size() != dim(). If relative_risks_transitions is not a dim() x dim() matrix.
            */
            OperatorMarkovModel(const FeatureUser<Feature>::feature_set_t& required,
                                const std::string& variable,
                                MarkovModel&& markov_model,
                                std::shared_ptr<const Predicate<T>> pred,
                                bool initialize,
                                Array2D<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks_transitions,
                                std::vector<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks_initial_state,
                                std::unique_ptr<Schedule>&& schedule);

            /** Dimension of the Markov model */
            unsigned int dim() const {
                return _markov_model.dim();
            }

            const Predicate<T>& predicate() const override {
                return *_pred;
            }
            
            void apply(const std::shared_ptr<T>& obj, const Contexts& contexts) const override;

            /** Apply a single transition without initialising the state
              @param rrvs Vector to store relative risk values in, must have size() == dim()
             */
            std::pair<Date, unsigned int> update_date_and_state(const T& obj, const Contexts& contexts, const std::pair<Date, unsigned int>& current_date_state, std::vector<double>& rrvs) const {
                const unsigned int from = current_date_state.second;                
                calc_relative_risks(obj, contexts, _relative_risks_transitions.at(from), rrvs);
                return _markov_model.select_next_state(current_date_state, rrvs, contexts.mutable_ctx().rng().next_uniform());
            }

            /** Get the last date for which the object has a well-defined state of the Markov process, and the state. */
            virtual std::pair<Date, unsigned int> get_last_date_and_state(const T& obj, const Contexts& ctx) const = 0;
            
            /** Is the object's Markov process initialized to some state */
            virtual bool is_initialized(const T& obj, const Contexts& contexts) const = 0;            

            /** Set the next state */
            virtual void set_next_state(T& obj, Date date, unsigned int state, const Contexts& ctx) const = 0;

            bool active(Date date) const override {
                return Operator<T>::active(_schedule, date);
            }
        protected:
            const std::string& variable() const {
                return _variable;
            }
        private:
            unsigned int draw_initial_state(const T& obj, const Contexts& contexts) const {
                std::vector<double> rrvs(dim()); // TODO: avoid repeated allocation                
                calc_relative_risks(obj, contexts, _relative_risks_initial_state, rrvs);
                return _markov_model.select_initial_state(rrvs, contexts.mutable_ctx().rng().next_uniform());
            }

            void calc_relative_risks(const T& obj, const Contexts& contexts, const std::vector<std::shared_ptr<const RelativeRisk<T>>>& rrs, std::vector<double>& values) const {
                assert(rrs.size() == values.size());
                std::transform(rrs.begin(), rrs.end(), values.begin(), [&obj, &contexts](const std::shared_ptr<const RelativeRisk<T>>& ptr) {
                        if (ptr) {
                            return (*ptr)(obj, contexts).relative_risk; // only value is relevant
                        } else {
                            return std::numeric_limits<double>::quiet_NaN();
                        }
                    });                
            }
            
            std::string _variable;
            MarkovModel _markov_model;
            std::shared_ptr<const Predicate<T>> _pred;
            bool _initialize;
            std::vector<std::shared_ptr<const RelativeRisk<T>>> _relative_risks_initial_state;
            Array2D<std::shared_ptr<const RelativeRisk<T>>> _relative_risks_transitions;
            std::unique_ptr<Schedule> _schedule;
        };
        
        template <class T> OperatorMarkovModel<T>::OperatorMarkovModel(const FeatureUser<Feature>::feature_set_t& required,
                                                                       const std::string& variable,
                                                                       MarkovModel&& markov_model,
                                                                       std::shared_ptr<const Predicate<T>> pred,
                                                                       bool initialize,
                                                                       Array2D<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks_transitions,
                                                                       std::vector<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks_initial_state,
                                                                       std::unique_ptr<Schedule>&& schedule)
        : OperatorIndividual<T>(false, FeatureUser<Feature>::feature_set_t({variable}),
                                FeatureUser<Feature>::combine(required, FeatureUser<Feature>::combine(FeatureUser<Feature>::gather_required_features(relative_risks_transitions, false), FeatureUser<Feature>::gather_required_features(relative_risks_initial_state, false)))
                                    ),
			_variable(variable), _markov_model(std::move(markov_model)), _pred(pred), _initialize(initialize) {
            if (!_pred) {
                throw std::domain_error("OperatorMarkovModel: null predicate");        
            }
            const size_t req_size = static_cast<size_t>(_markov_model.dim());
            if (relative_risks_initial_state.size() != req_size) {
                throw std::domain_error("OperatorMarkovModel: set_relative_risks_initial_state: bad input size");
            }
            if (relative_risks_transitions.size() != req_size) {
                throw std::domain_error("OperatorMarkovModel: set_relative_risks_transitions: bad input size");
            }
            for (const auto& row : relative_risks_transitions) {
                if (row.size() != req_size) {
                    throw std::domain_error("OperatorMarkovModel: set_relative_risks_transitions: bad input row size");
                }
            }
            _relative_risks_transitions = std::move(relative_risks_transitions);
            _relative_risks_initial_state = std::move(relative_risks_initial_state);
            _schedule = std::move(schedule);
        }

        template <class T> void OperatorMarkovModel<T>::apply(const std::shared_ptr<T>& obj, const Contexts& contexts) const {
            assert(obj);
            assert(!_schedule || contexts.immutable_ctx().schedule().contains(*_schedule));
            if (is_initialized(*obj, contexts)) {
                std::pair<Date, unsigned int> date_and_state(get_last_date_and_state(*obj, contexts));
                const SchedulePeriod sp = Operator<T>::current_period(_schedule, contexts);
                std::vector<double> rrvs(dim()); // TODO: avoid repeated allocation
                // cover the next schedule period
                while (date_and_state.first < sp.end) {                    
                    date_and_state = update_date_and_state(*obj, contexts, date_and_state, rrvs);
                    set_next_state(*obj, date_and_state.first, date_and_state.second, contexts);
                }
            } else {
                if (_initialize) {
                    const unsigned int state = draw_initial_state(*obj, contexts);
                    set_next_state(*obj, contexts.asof(), state, contexts);
                }
            }
        }
        
    }
}

#endif // __AVERISERA_MICROSIM_OPERATOR_MARKOV_MODEL_HPP
