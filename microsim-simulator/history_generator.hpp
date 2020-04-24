#ifndef __AVERISERA_MICROSIM_HISTORY_GENERATOR_H
#define __AVERISERA_MICROSIM_HISTORY_GENERATOR_H

#include "history_factory.hpp"
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace averisera {
    namespace microsim {
        class History;
        template <class T> class Predicate;
        
        /*! An object which generates History stored on all or selected objects of class T */
        template <class T> class HistoryGenerator {
        public:
            typedef HistoryFactory::factory_t factory_t;
            
            typedef std::tuple<std::string, factory_t, std::shared_ptr<const Predicate<T> > > req_t; /*!< Requirement type: (name, factory, predicate) */
            typedef std::vector<req_t> reqvec_t; /*!< Vector of requirements */

            /*! Return History requirements of this generator.
              Each requirement is a tuple of (name, history factory, predicate). Factory cannot be null.
              Predicate selects the objects on which histories created by given factory and for given variable are to be stored.
             */
			virtual const reqvec_t& requirements() const = 0;

			static const reqvec_t& EMPTY() {
				static const reqvec_t empty;
				return empty;
			}
        };

        
    }
}

#endif // __AVERISERA_MICROSIM_HISTORY_GENERATOR_H
