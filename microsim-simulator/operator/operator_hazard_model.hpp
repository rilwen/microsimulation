// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_OPERATOR_HAZARD_MODEL_H
#define __AVERISERA_MS_OPERATOR_HAZARD_MODEL_H

#include "../operator_individual.hpp"
#include "microsim-core/hazard_model.hpp"
#include "../relative_risk.hpp"
#include "../relative_risk_factory.hpp"
#include <cassert>
#include <set>
#include <stdexcept>
#include <string>

namespace averisera {
    namespace microsim {
        /** Operator which uses a HazardModel to update a state variable. Transitions can happen between the dates of the schedule */
        template <class T> class OperatorHazardModel: public OperatorIndividual<T> {
        public:            
            /**
             * @param[in] provided_feature Feature provided by this operator
             * @param[in] hazard_model Hazard model for base probability
             * @param[in] relative_risks Vector of relative risks, resized internally to hazard_model.dim(). Any null pointer will be replaced by RelativeRisk implementation which always returns 1.
             * @param[in] predicate Predicate for the operator
             @param schedule Custom schedule (moved). Must be contained in context schedule. Null to use context schedule.
             * @throw std::domain_error If predicate is null.
             */
            OperatorHazardModel(const Feature& provided_feature, const HazardModel& hazard_model, const std::vector<std::shared_ptr<const RelativeRisk<T>>>& relative_risks,
                                std::shared_ptr<const Predicate<T>> predicate, std::unique_ptr<Schedule>&& schedule);

			/** Constructor which moves more elements */
			OperatorHazardModel(const Feature& provided_feature, HazardModel&& hazard_model, std::vector<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks,
				std::shared_ptr<const Predicate<T>> predicate, std::unique_ptr<Schedule>&& schedule);

			const Predicate<T>& predicate() const {
                return *_pred;
            }
            
            void apply(const std::shared_ptr<T>& obj, const Contexts& contexts) const override;

            bool active(Date date) const override {
                return Operator<T>::active(_schedule, date);
            }
        protected:
			typedef unsigned int state_t;
			const HazardModel& hazard_model() const {
				return _hazard_model;
			}
		private:
            /** Get the current state of the object */
            virtual state_t current_state(const T& obj, const Contexts& ctx) const = 0;
            /** Set the next state */
            virtual void set_next_state(T& obj, Date date, state_t state, const Contexts& ctx) const = 0;
			/** If returns null, use _hazard_model */
			virtual std::unique_ptr<HazardModel> adapt_hazard_model(const T& /*obj*/) const {
				return nullptr;
			}
            
            HazardModel _hazard_model;
            std::vector<std::shared_ptr<const RelativeRisk<T>>> _relative_risks;
            std::shared_ptr<const Predicate<T>> _pred;
            std::unique_ptr<Schedule> _schedule;
        };
        
        template <class T> OperatorHazardModel<T>::OperatorHazardModel(const Feature& provided_feature, const HazardModel& hazard_model, const std::vector<std::shared_ptr<const RelativeRisk<T>>>& relative_risks, std::shared_ptr<const Predicate<T>> predicate, std::unique_ptr<Schedule>&& schedule)
            : OperatorIndividual<T>(false, FeatureUser<Feature>::feature_set_t({provided_feature}), FeatureUser<Feature>::gather_required_features(relative_risks, false)), _hazard_model(hazard_model), _relative_risks(relative_risks), _pred(predicate) {
			_relative_risks.resize(_hazard_model.dim());
            for (std::shared_ptr<const RelativeRisk<T>>& rr: _relative_risks) {
                if (!rr) {
                    rr = RelativeRiskFactory::make_constant<T>(RelativeRiskValueUnbound());
                }
            }
            _schedule = std::move(schedule);
        }

		template <class T> OperatorHazardModel<T>::OperatorHazardModel(const Feature& provided_feature, HazardModel&& hazard_model, std::vector<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks,
			std::shared_ptr<const Predicate<T>> predicate, std::unique_ptr<Schedule>&& schedule)
			: OperatorIndividual<T>(false, FeatureUser<Feature>::feature_set_t({ provided_feature }), FeatureUser<Feature>::gather_required_features(relative_risks, false)), _hazard_model(std::move(hazard_model)), _relative_risks(std::move(relative_risks)), _pred(predicate) {
			_relative_risks.resize(_hazard_model.dim());
			for (std::shared_ptr<const RelativeRisk<T>>& rr : _relative_risks) {
				if (!rr) {
					rr = RelativeRiskFactory::make_constant<T>(RelativeRiskValueUnbound());
				}
			}
			_schedule = std::move(schedule);
		}
        
        template <class T> void OperatorHazardModel<T>::apply(const std::shared_ptr<T>& objptr, const Contexts& contexts) const {
            assert(!_schedule || contexts.immutable_ctx().schedule().contains(*_schedule));
            T& obj = *objptr;
            Date date = contexts.asof(); //
            const SchedulePeriod sp = Operator<T>::current_period(_schedule, contexts);            
            unsigned int state = current_state(obj, contexts);            
			const std::unique_ptr<HazardModel> adapted_hazard_model(adapt_hazard_model(obj));
			const HazardModel& hm = adapted_hazard_model ? *adapted_hazard_model : _hazard_model;
            while (date < sp.end) {
                const RelativeRiskValue rrv = (*(_relative_risks[state]))(obj, contexts);
                const HazardRateMultiplier hazard_rate_multiplier = hm.calc_hazard_rate_multiplier(state, rrv);
                const double jump_proba = hm.calc_transition_probability(state, date, sp.end, hazard_rate_multiplier);
                const double u = contexts.mutable_ctx().rng().next_uniform();
                const Date jump_date = hm.calc_end_date(state, date, u, hazard_rate_multiplier);
                if (u < jump_proba && jump_date < sp.end) {
                    ++state;
                    set_next_state(obj, jump_date, state, contexts);
                }
                date = jump_date;
            }
        }
    }
}

#endif // __AVERISERA_MS_OPERATOR_HAZARD_MODEL_H
