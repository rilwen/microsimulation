/*
(C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_HISTORY_H
#define __AVERISERA_MS_HISTORY_H

#include "immutable_history.hpp"
#include "core/dates.hpp"

namespace averisera {
    namespace microsim {
        /*! \brief Abstract base class for all implementations of history.
         */
        class History: public ImmutableHistory {
        public:
            virtual ~History();
            /*! Append new double value to history.
             *
             * \param[in] date New date, must be past the last date
             * \param[in] value New value
             * \throws std::domain_error if new date not past the current last date
             \throws std::out_of_range If the value appended does not fit within the range of the values supported by the history
             */
            virtual void append(Date date, double_t value) = 0;

            /*! Append new int_t value to history.
             *
             * \param[in] date New date, must be past the last date
             * \param[in] value New value
             * \throws std::domain_error if new date not past the current last date
             \throws std::out_of_range If the value appended does not fit within the range of the values supported by the history
             */
            virtual void append(Date date, int_t value) = 0;

            // Synonym to make writing tests easier.
            void append_int(Date date, int_t value) {
                this->append(date, value);
            }

            /*! Append (if the last date is before date or history is empty) or correct (if it's equal) a value in history.
             * \param[in] date New date, must not be before the last date
             * \param[in] value New value
             * \throw std::domain_error If date before last date.
             */
            void append_or_correct(Date date, double_t value);
        
            /*! Correct the last double_t value in history. Used when enforcing a distribution.
             * \param[in] value New value
             * \throw std::domain_error If history is empty.
             */
            virtual void correct(double_t value) = 0;

            /*! Deep clone of History object */
            virtual std::unique_ptr<History> clone() const = 0;
        };
    }
}

#endif // __AVERISERA_MS_HISTORY_H
