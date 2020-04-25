#ifndef __AVERISERA_MS_FEATURE_USER_H
#define __AVERISERA_MS_FEATURE_USER_H

#include <algorithm>
#include <memory>
#include <unordered_set>
#include <vector>

namespace averisera {
    namespace microsim {
        /** Class which uses abstract features
         * @tparam F Feature type
         */
        template <class F> class FeatureUser {
        public:
            /** Virtual destructor because this class will be extended by classes implementing the operations */
			virtual ~FeatureUser();

            typedef std::unordered_set<F> feature_set_t; /**< Set of features */
            
            /** @brief Features required
             \ return Reference to set of* features required
             */
            virtual const feature_set_t& requires() const = 0;
            
			static feature_set_t combine(const feature_set_t& first, const feature_set_t& second);

            template <class C> static feature_set_t gather_required_features(const C& containers, bool require_not_null) {
                feature_set_t features;
                for (const auto& c: containers) {
                    const feature_set_t new_features(gather_required_features(c, require_not_null));
                    features.insert(new_features.begin(), new_features.end());
                }
                return features;
            }

            template <class T> static feature_set_t gather_required_features(const std::vector<std::shared_ptr<T>>& users, bool require_not_null) {
                feature_set_t features;
                for (const std::shared_ptr<T>& ptr: users) {
                    if (ptr) {
                        const feature_set_t& new_features = ptr->requires();
                        features.insert(new_features.begin(), new_features.end());
                    } else {
                        if (require_not_null) {
                            throw std::domain_error("FeatureUser: null user pointer");
                        }
                    }
                    
                }
                return features;
            }
        };
    }
}

#endif // __AVERISERA_MS_FEATURE_USER_H
