/*
 * (C) Averisera Ltd 2015
 */
#include "archived_history.hpp"
#include <cassert>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        ArchivedHistory::ArchivedHistory()
        : _actor_id(0), _variable_id(0) {
        }
        
        ArchivedHistory::ArchivedHistory(unsigned int actor_id, unsigned int variable_id, const History* history)
        : _actor_id(actor_id), _variable_id(variable_id), _history(history) {
            assert(actor_id > 0);
            assert(variable_id > 0);
            assert(history != nullptr);
        }
        
        const History& ArchivedHistory::history() const {
            if (_history != nullptr) {
                return *_history;
            } else {
                throw std::logic_error("No history");
            }
        }
    }
}