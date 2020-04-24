/*
  (C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_PREDICATE_H
#define __AVERISERA_MS_PREDICATE_H

#include <cassert>
#include <memory>
#include <vector>
#include "core/dates.hpp"
#include "core/printable.hpp"

namespace averisera {
    namespace microsim {
        class Contexts;
                
        /*! \brief Selects an object based on some criteria.
         * 
         * While it may require setting some properties of the object before applying,
         * it is not a FeatureUser.

         Predicate implementations should be immutable.

		 TODO: Predicate should return a list of variables it depends on which would be added to the HistoryUser requirements returned by Operator which uses the Predicate

         \tparam T Object type
        */
        template <class T> class Predicate: public Printable {
        public:
            virtual ~Predicate() {}

            /*! Decide whether object is selected or not.

              \param[in] obj Object
              \param[in] contexts Contexts object with additional shared data.

              \return True if selected.
            */
            virtual bool select(const T& obj, const Contexts& contexts) const = 0;

			/*! Select or not, assuming obj is "alive" */
			virtual bool select_alive(const T& obj, const Contexts& contexts) const {
				return select(obj, contexts);
			}

			/*! Ignore null pointers 
			\tparam U T or const T */
			template <class U> void select(const std::vector<std::shared_ptr<U>>& objects, const Contexts& contexts, std::vector<std::shared_ptr<U>>& selected) const {
				for (const auto& ptr : objects) {
					if (ptr && select(*ptr, contexts)) {
						selected.push_back(ptr);
					}
				}
			}

			/*! Select assuming every object is "alive". Ignore null pointers.
			\tparam U T or const T */
			template <class U> void select_alive(const std::vector<std::shared_ptr<U>>& objects, const Contexts& contexts, std::vector<std::shared_ptr<U>>& selected) const {
				for (const auto& ptr : objects) {
					if (ptr && select_alive(*ptr, contexts)) {
						selected.push_back(ptr);
					}
				}
			}

			/*! Is the Predicate selecting any objects on this date? 
			Must be compatible with the select methods. */
			virtual bool active(Date) const {
				return true;
			}

            /*! Select the object without any context information such as simulation date,
              based on its basic properties only. Perform "wide" selection, i.e. if the predicate select() method
              selects objects based on context-sensitive properties (such as current age, current value of variable),
              assume that the context-sensitive selection criteria are met.

              Any object selected by select() must also be selected by select_out_of_context().
            */
            virtual bool select_out_of_context(const T& obj) const = 0;
            
            /*! Whether select() always returns true. */
            virtual bool always_true() const {
                return false;
            }

            /*! Whether select_out_of_context() always returns true. 
              If always_true() == true, then always_true_out_of_context() must be true as well.
             */
            virtual bool always_true_out_of_context() const {
                return false;
            }

			/*! Selects only entities which are "alive" (e.g. live persons, active companies, etc.) */
			virtual bool selects_alive_only() const {
				return false;
			}
            
            /*! Create deep copy of the Predicate. Caller is expected to manage the pointer.
             */
            virtual Predicate<T>* clone() const = 0;

            /*! Create a new Predicate which selects a union of objects selected by this or other Predicate.
              Either Predicate may be copied to achieve that.
              \throw std::domain_error If other is null
            */
            virtual std::shared_ptr<const Predicate<T> > sum(std::shared_ptr<const Predicate<T> > other) const;

            /*! Create a new Predicate which selects an intersection of objects selected by this or other Predicate.
              Either Predicate may be copied to achieve that.
              \throw std::domain_error If other is null
            */
            virtual std::shared_ptr<const Predicate<T> > product(std::shared_ptr<const Predicate<T> > other) const;

            /*! Create a new Predicate which negates the current one.
             */
            virtual std::shared_ptr<const Predicate<T>> negate() const;
        
            virtual std::shared_ptr<const Predicate<T> > sum(const Predicate<T>& other) const {
                const auto cloned = other.clone();
                assert(cloned);
                return sum(std::shared_ptr<const Predicate<T>>(cloned));
            }

            std::shared_ptr<const Predicate<T> > product(const Predicate<T>& other) const {
                const auto cloned = other.clone();
                assert(cloned);
                return product(std::shared_ptr<const Predicate<T>>(cloned));
            }			
        };
    }
}

#include "predicate/predicate_calculus.hpp"

// Default implementations

namespace averisera {
    namespace microsim {
        template <class T> std::shared_ptr<const Predicate<T> > Predicate<T>::sum(std::shared_ptr<const Predicate<T> > other) const {
            // make_shared doesn't work with initializer lists
            return std::shared_ptr<Predicate<T>>(new PredOr<T>({std::shared_ptr<Predicate<T> >(this->clone()), other}));
        }
        
        template <class T> std::shared_ptr<const Predicate<T> > Predicate<T>::product(std::shared_ptr<const Predicate<T> > other) const {
            return std::shared_ptr<Predicate<T>>(new PredAnd<T>({std::shared_ptr<Predicate<T> >(this->clone()), other}));
        }
        
        template <class T> std::shared_ptr<const Predicate<T> > Predicate<T>::negate() const {
            return std::make_shared<const PredNot<T> >(std::shared_ptr<const Predicate<T> >(clone()));
        }
    }
}


#endif // __AVERISERA_MS_PREDICATE_H
