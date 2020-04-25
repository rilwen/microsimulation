#ifndef __AVERISERA_MICROSIM_HISTORY_USER_HPP
#define  __AVERISERA_MICROSIM_HISTORY_USER_HPP

#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace averisera {
    namespace microsim {
        template <class T> class Predicate;
        
        /** An object which uses (to read or write) History stored on all or selected objects of class T.
          Note that objects which generate Histories should implement HistoryGenerator<T>.
          Adding generated histories to the list of requirements of HistoryUser is not an error, but is unnecessary.
         */
        template <class T> class HistoryUser {
        public:
            typedef std::string use_req_t; /**< Requirement type: name of history */
            typedef std::vector<use_req_t> use_reqvec_t; /**< Vector of requirements */

            virtual ~HistoryUser() {
            }

            /** Return History requirements of this user 
              Each requirement is a tuple of (name, predicate).
              Predicate selects the objects on which histories for given variable are to be stored.
             */
			virtual const use_reqvec_t& user_requirements() const = 0;

			static const use_reqvec_t& EMPTY() {
				static const use_reqvec_t empty;
				return empty;
			}
        };
    }
}

#endif //  __AVERISERA_MICROSIM_HISTORY_USER_HPP
