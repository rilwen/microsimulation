#ifndef __AVERISERA_MS_DISPATCHER_H
#define __AVERISERA_MS_DISPATCHER_H

#include "feature.hpp"
#include "feature_user.hpp"
#include <memory>

namespace averisera {
    namespace microsim {
        class Contexts;
        template <class T> class Predicate;
        
        /*! Given an argument, dispatches it to correct handler 
         * \tparam A Argument type
         * \tparam H Handler type
         */
        template <class A, class H> class Dispatcher: public FeatureUser<Feature> {
        public:
            virtual ~Dispatcher() {}
            
            /*! Get the handler for argument 
             * \param[in] obj Object for which predicate->select(obj, contexts) == true
             * \param[in] contexts Contexts
             * \return Appropriate handler
             * \throw std::domain_error If argument cannot be handled.
             */
            virtual H dispatch(const A& obj, const Contexts& contexts) const = 0;

            /*! Get the handler for argument 
             * \param[in] obj Object for which predicate->select_out_of_context(obj) == true
             * \return Appropriate handler - due to lack of context information, it might not be the most appropriate one. It can also be a different handler than
             any of the ones returned by dispatch().
             * \throw std::domain_error If argument cannot be handled.
             */
            virtual H dispatch_out_of_context(const A& obj) const = 0;

            /*! Return a non-null predicate which selects objects handled by the dispatcher */
            virtual std::shared_ptr<const Predicate<A> > predicate() const = 0;
        };
    }
}

#endif // __AVERISERA_MS_DISPATCHER_H
