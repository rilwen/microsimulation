/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_ARCHIVED_HISTORY_H
#define __AVERISERA_MS_ARCHIVED_HISTORY_H

#include <memory>
#include "history.hpp"

namespace averisera {
    namespace microsim {
        /*! \brief Stores archived History.
         * 
         * When we remove an Actor from memory, we may want to keep some of their histories. The history is then moved to this object.
         * ArchivedHistory takes over the ownership of the History object.
         * 
         * \see History
         */
        class ArchivedHistory {
        public:
            /*! Construct empty history */
            ArchivedHistory();
            
            /*! \brief Construct an archived history.
             * 
             * Takes over the ownership of the History object.
             * \param actor_id ID of the Actor whose history is archived
             * \param variable_id ID of the variable which history is archived
             * \param histor Pointer to History, not null.
             */
            ArchivedHistory(unsigned int actor_id, unsigned int variable_id, const History* history);
            
            /*! Return ID of the Actor whose history is archived
             */
            unsigned int actor_id() const {
                return _actor_id;
            }
            
            /*! Return ID of the variable which history is archived */
            unsigned int variable_id() const {
                return _variable_id;
            }
            
            /*! Check if we have any history in store. */
            bool empty() const {
                return _history == nullptr;
            }
            
            /*! Return archived history.
             * \throw std::logic_error If empty
             */
            const History& history() const;
        private:
            unsigned int _actor_id;
            unsigned int _variable_id;
            std::unique_ptr<const History> _history;
        };
    }
}

#endif // __AVERISERA_MS_ARCHIVED_HISTORY_H