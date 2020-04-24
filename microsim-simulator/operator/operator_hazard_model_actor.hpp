#ifndef __AVERISERA_MS_OPERATOR_HAZARD_MODEL_ACTOR_H
#define __AVERISERA_MS_OPERATOR_HAZARD_MODEL_ACTOR_H

#include "operator_hazard_model.hpp"
//#include "../actor.h"
#include "../history_generator_simple.hpp"
//#include <type_traits>

namespace averisera {
    namespace microsim {
        class HistoryRegistry;
        
        /*! OperatorHazardModel which acts on Actor or derived objects
         \tparam T Actor or class derived from it
        */
        template <class T> class OperatorHazardModelActor: public OperatorHazardModel<T> {
            //static_assert(std::is_base_of<Actor, T>::value, "T must be derived from Actor");
        public:
            /* \see OperatorHazardModel
            \param[in] state_variable Name of the state variable
            */
            OperatorHazardModelActor(const Feature& provided_feature, const HazardModel& hazard_model
                                     , const std::vector<std::shared_ptr<const RelativeRisk<T>>>& relative_risks
                                     , std::shared_ptr<const Predicate<T>> predicate, const std::string& state_variable
                                     , HistoryFactory::factory_t history_factory, std::unique_ptr<Schedule>&& schedule);

            /*! Get current state from a Poisson process history */
            static unsigned int current_state(const T& obj, const Contexts& ctx, const std::string& state_variable);

            /*! Get next state in a Poisson process history */
            static void set_next_state(T& obj, Date date, unsigned int state, const Contexts& ctx, const std::string& state_variable);

			const typename HistoryGenerator<T>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const std::string& name() const override {
				static const std::string str("HazardModelActor");
				return str;
			}
        private:
            unsigned int current_state(const T& obj, const Contexts& ctx) const override;
            void set_next_state(T& obj, Date date, unsigned int state, const Contexts& ctx) const override;

			HistoryGeneratorSimple<T> hist_gen_;
            std::string _state_variable;
        };
    }
}

#endif // __AVERISERA_MS_OPERATOR_HAZARD_MODEL_ACTOR_H
