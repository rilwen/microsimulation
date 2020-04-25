#ifndef __AVERISERA_MS_FEATURE_PROVIDER_H
#define __AVERISERA_MS_FEATURE_PROVIDER_H

#include "feature_user.hpp"
#include "core/log.hpp"
#include "core/sorting.hpp"
#include "core/stl_utils.hpp"
#include <cassert>
#include <memory>
#include <type_traits>
#include <unordered_set>
#include <algorithm>

namespace averisera {
    namespace microsim {
        /** Provides or requires features. Used to sort out in what order should we apply operations to data. 
         *	  @tparam F Feature class
         */
        template <class F> class FeatureProvider: public FeatureUser<F> {
        public:
            typedef typename FeatureUser<F>::feature_set_t features_set_t;

            /** @brief Features provided
             *            @return Reference to set sorted in ascending order
             */
            virtual const features_set_t& provides() const = 0;
            
            /** Calculate the relation between this and another provider.
             *            @return -1 if this provider provides Features required by the other provider;
             *            1 if the other provider provides Features required by this provider;
             *            0 otherwise.
             *            @throw std::runtime_error If relation cannot be calculated.
             */
            virtual int relation(const FeatureProvider<F>& other) const;
            
            /** Sort a vector of pointers to feature providers by building a graph of 
             *            their relations and sorting it according to who requires whom.
             *	    @tparam I Class derived from FeatureProvider<F>
             *            @throw std::runtime_error If the relations graph has cycles (e.g. A requires B, B requires C, C requires A)
             */
            template <class I> static void sort(std::vector<std::shared_ptr<I>>& providers) {
                static_assert(std::is_base_of<FeatureProvider<F>, I>::value, "I must be derived from FeatureProvider<F>");
                Sorting::topological_sort(providers, [](const std::shared_ptr<I>& a, const std::shared_ptr<I>& b) -> int {
                    assert(a);
                    assert(b);
					return a->relation(*b);
                });
            }
            
            /** Process provided and required features:
             *	      2. make sure there are no duplicates
             *	      3. make sure there are no features which are both provided and required
             */
            static void process_features(features_set_t& provided, features_set_t& required);
            
            /** Check if all non-optional requirements are satisfied. Assumes the relationship graph is not cyclic.
			@param ignored Set of requirements which can be ignored when checking
			@param always_required Additional requirements which have to be met
             * @tparam I Class derived from FeatureProvider<F>
             */
            template <class I> static bool are_all_requirements_satisfied(const std::vector<std::shared_ptr<I>>& providers, const features_set_t& ignored, const features_set_t& always_required) {
                static_assert(std::is_base_of<FeatureProvider<F>, I>::value, "I must be derived from FeatureProvider<F>");
                features_set_t provided;
                features_set_t required(always_required);
                std::for_each(providers.begin(), providers.end(), [&provided, &required](const std::shared_ptr<I>& provider) {
                    provided.insert(provider->provides().begin(), provider->provides().end());
                    required.insert(provider->requires().begin(), provider->requires().end());                    
                });
				for (const F& req : required) {
					if (ignored.find(req) == ignored.end()) {
						const bool found = provided.find(req) != provided.end();
						if (!found) {
							LOG_WARN() << "FeatureProvider: requirement " << req << " not satisfied";
							LOG_DEBUG() << "FeatureProvider: All provided features: " << provided;
							LOG_DEBUG() << "FeatureProvider: always_required: " << always_required;
							LOG_DEBUG() << "FeatureProvider: ignored: " << ignored;
							return false;
						}
					}
				}
				return true;
            }
        };
        
        template <class F> int FeatureProvider<F>::relation(const FeatureProvider<F>& other) const {
            bool provides = false;
            bool requires = false;
            const auto& f1p = this->provides();
            const auto& f1r = this->requires();
            const auto& f2p = other.provides();
            const auto& f2r = other.requires();
            for (auto it = f1p.begin(); it != f1p.end(); ++it) {
                provides |= (f2r.find(*it) != f2r.end());
            }
            for (auto it = f2p.begin(); it != f2p.end(); ++it) {
                requires |= (f1r.find(*it) != f1r.end());
            }
            int result = 0;
            if (provides) {
                if (!requires) {
                    result = -1;
                } else {
                    throw std::runtime_error("Operator: cannot calculate relation");
                }
            } else if (requires) {
                result = 1;
            }
            //LOG_TRACE() << "FeatureProvider::relation: returning " << result;
            return result;
        }
        
        template <class F> void FeatureProvider<F>::process_features(features_set_t& provided, features_set_t& required) {
            features_set_t intersection;
            std::set_intersection(provided.begin(), provided.end(),
                                  required.begin(), required.end(),
                                  std::inserter(intersection, intersection.begin()));
            if (! intersection.empty()) {
				LOG_ERROR() << "FeatureProvider<F>::process_features: features " << intersection << " are both provided and required";
                throw std::domain_error("FeatureProvider: features both provided and required");
            }
        }
    }
}
    
#endif // __AVERISERA_MS_FEATURE_PROVIDER_H
    
