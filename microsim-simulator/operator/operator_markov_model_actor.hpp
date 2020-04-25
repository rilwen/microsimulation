#ifndef __AVERISERA_MICROSIM_OPERATOR_MARKOV_MODEL_ACTOR_HPP
#define __AVERISERA_MICROSIM_OPERATOR_MARKOV_MODEL_ACTOR_HPP

//#include "../actor.h"
#include "../history_generator_simple.hpp"
#include "operator_markov_model.hpp"
//#include <type_traits>

namespace averisera {
    namespace microsim {
        /** Acts on Actor or derived classes */
        template <class T> class OperatorMarkovModelActor: public OperatorMarkovModel<T> {
            //static_assert(std::is_base_of<Actor, T>::value, "T must be derived from Actor");
        public:
            /** 
			@param history_factory History factory for storing the state (must be int type)
			@see OperatorMarkovModel */
            OperatorMarkovModelActor(const FeatureUser<Feature>::feature_set_t& required,
                                     const std::string& variable,
                                     MarkovModel&& markov_model,
                                     std::shared_ptr<const Predicate<T>> pred,
                                     bool initialize,
                                     Array2D<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks_transitions,
                                     std::vector<std::shared_ptr<const RelativeRisk<T>>>&& relative_risks_initial_state,
                                     std::unique_ptr<Schedule>&& schedule,
				HistoryFactory::factory_t history_factory
				);
            bool is_initialized(const T& obj, const Contexts& contexts) const override;
            std::pair<Date, unsigned int> get_last_date_and_state(const T& obj, const Contexts& ctx) const override;
            void set_next_state(T& obj, Date date, unsigned int state, const Contexts& ctx) const override;

			const typename HistoryGenerator<T>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const std::string& name() const override {
				static const std::string str("MarkovModelActor");
				return str;
			}
		private:
			HistoryGeneratorSimple<T> hist_gen_;
        };
    }
}

#endif // __AVERISERA_MICROSIM_OPERATOR_MARKOV_MODEL_ACTOR_HPP
