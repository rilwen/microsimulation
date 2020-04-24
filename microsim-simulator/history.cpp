/*
(C) Averisera Ltd 2015
*/
#include "history.hpp"
#include "core/log.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        History::~History() {
        }
        
        void History::append_or_correct(Date date, double value) {
            if (empty()) {
                append(date, value);
            } else {
                const Date ld = last_date();
                if (date < ld) {
					LOG_ERROR() << "History::append_or_correct: date " << date << " before last date " << ld;
                    throw std::domain_error("History: date before last date");
                }
                if (ld == date) {
                    correct(value);
                } else {
                    append(date, value);
                }
            }
        }
    }
}
