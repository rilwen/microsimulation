#ifndef __AVERISERA_MS_OPERATOR_INDIVIDUAL_H
#define __AVERISERA_MS_OPERATOR_INDIVIDUAL_H

#include "operator.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        /** @brief Operator acting on each object independently */
        template <class T> class OperatorIndividual: public Operator<T> {
        public:
            /** @see Operator */
            OperatorIndividual(bool is_instantaneous, const FeatureUser<Feature>::feature_set_t& provided, const FeatureUser<Feature>::feature_set_t& required)
                : Operator<T>(is_instantaneous, provided, required)
                {}
            
			/** @see Operator */
			OperatorIndividual(bool is_instantaneous, const FeatureUser<Feature>::feature_set_t& provided)
				: Operator<T>(is_instantaneous, provided)
			{}

            /** @see Operator */
            OperatorIndividual(bool is_instantaneous)
                : Operator<T>(is_instantaneous)
                {}
            
            void apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const {
                for (auto it = selected.begin(); it != selected.end(); ++it) {
                    if (*it) {
                        apply(*it, contexts);
                    } else {
                        throw std::domain_error("OperatorIndividual: null pointer");
                    }
                }
            }

            /** @param obj Not-null pointer to object T
             */
            virtual void apply(const std::shared_ptr<T>& obj, const Contexts& contexts) const = 0;
        };
    }
}

#endif // __AVERISERA_MS_OPERATOR_INDIVIDUAL_H
