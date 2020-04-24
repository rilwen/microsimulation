#ifndef __AVERISERA_MS_OPERATOR_H
#define __AVERISERA_MS_OPERATOR_H

#include <memory>
#include <set>
#include <string>
#include "contexts.hpp"
#include "feature.hpp"
#include "feature_provider.hpp"
#include "history_generator.hpp"
#include "history_user.hpp"
#include "microsim-core/schedule.hpp"

namespace averisera {
    namespace microsim {
        
        class Contexts;
        
        template <class T> class Predicate;

        /*! Operator which acts instantaneously on asof date */
        const Feature& INSTANTANEOUS(); 
        
        /*! \brief Operates on objects of given class.
         * 
         *	  An instantaneous operator sets a property of the Actor, e.g. the BMI of a Person at date T_i.
         *	  A non-instantaneous operator acts over dates T_i <= d < T_{i+1}, e.g. causing the Person to develop cancer
         *	  or die.
		 *    Non-instantanenous operators REQUIRE instantaneous operators to act first.
         *	  
         *	  Feature requirements of an Operator should include also the requirements of its Predicate.
         *	  
         *
         * \tparam T Object class (e.g. Person).
         */
        template <class T> class Operator: public FeatureProvider<Feature>, public HistoryGenerator<T>, public HistoryUser<T> {
        public:
            Operator(bool is_instantaneous)
                : Operator(is_instantaneous, Feature::empty(), Feature::empty()) {}

			Operator(bool is_instantaneous, const FeatureUser<Feature>::feature_set_t& provided)
				: Operator(is_instantaneous, provided, Feature::empty()) {}
            
            /*!
             *	      Features cannot contain INSTANTANEOUS and cannot contain duplicates. No feature can be provided and
             *	      required simultaneously.
             *	      All required features are treated as non-optional.
             *	      \see FeatureProvider
             */
            Operator(bool is_instantaneous, const FeatureUser<Feature>::feature_set_t& provided, const FeatureUser<Feature>::feature_set_t& required)
                : _is_instantaneous(is_instantaneous), _provided(provided), _required(required) {
                if (_is_instantaneous) {
                    _provided.insert(INSTANTANEOUS());
                } else {
                    _required.insert(INSTANTANEOUS());                    
                }		
                FeatureProvider<Feature>::process_features(_provided, _required);
            }
            
            virtual ~Operator() {}
            
            /*! Return the predicate which selects which objects to act on. 
              The predicate will be invoked only on dates d for which active(d) returns true.
             */
            virtual const Predicate<T>& predicate() const = 0;
            
            /*! Act on selected objects. Invoked only when active(contexts.asof()) return true.
             * \param selected A non-empty vector
             \param contexts Contexts.
             * \throw std::domain_error If any pointer in selected is null.
             */
            virtual void apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const = 0;

			bool is_active(Date date) const {
				return active(date) && predicate().active(date);
			}
            
            /*! Whether the operator acts instantaneously or over the period till next simulation date. */
            bool is_instantaneous() const {
                return _is_instantaneous;
            }
            
            const FeatureUser<Feature>::feature_set_t& provides() const override {
                return _provided;
            }
            
			/*! Features returned should also include the requirements of the Predicate */
            const FeatureUser<Feature>::feature_set_t& requires() const override {
                return _required;
            }

			const typename HistoryGenerator<T>::reqvec_t& requirements() const override {
				return HistoryGenerator<T>::EMPTY();
			}

			/*! HistoryUser requirements returned should also include the requirements of the Predicate */
			const typename HistoryUser<T>::use_reqvec_t& user_requirements() const override {
				return HistoryUser<T>::EMPTY();
			}

			virtual const std::string& name() const = 0;
        protected:
            static bool active(const std::unique_ptr<Schedule>& schedule, Date date) {
                if (schedule) {
                    return schedule->contains(date);
                } else {
                    return true;
                }
            }

            static SchedulePeriod current_period(const std::unique_ptr<Schedule>& schedule, const Contexts& contexts) {
                if (schedule) {
                    const Schedule::index_t date_idx = schedule->index(contexts.asof());
					if (date_idx < schedule->size()) {
						return (*schedule)[date_idx];
					} else {
						return SchedulePeriod(contexts.asof());
					}
                } else {
                    return contexts.current_period();
                }
            }
        private:
			/*! Is the operator active on given date?
			\param date Date from context schedule
			*/
			virtual bool active(Date date) const {
				return true;
			}

            bool _is_instantaneous;
            FeatureUser<Feature>::feature_set_t _provided;
            FeatureUser<Feature>::feature_set_t _required;
        };
        
    }
}

#endif // __AVERISERA_MS_OPERATOR_H
