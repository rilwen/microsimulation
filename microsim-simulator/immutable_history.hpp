// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_IMMUTABLE_HISTORY_H
#define __AVERISERA_MS_IMMUTABLE_HISTORY_H

#include "history_data.hpp"
#include "core/dates_fwd.hpp"
#include "core/printable.hpp"
#include <cstdint>


namespace averisera {
    namespace microsim {
        /** Abstract base class for all implementations of read-only history.
          History is a number of events. Each event is a (date, value) pair. Values can be retrieved as double or as int.          
         */
        class ImmutableHistory: public Printable {
        public:
            typedef double double_t; /**< Double precision type supported by ImmutableHistory */
            typedef long int int_t; /**< Integer type supported by ImmutableHistory. Has to be long int to avoid ambiguous overload problem. */
            typedef size_t index_t; /**< Type used to index values */
            
            virtual ~ImmutableHistory();
			ImmutableHistory() = default;
			ImmutableHistory(const ImmutableHistory&) = delete;
			ImmutableHistory& operator=(const ImmutableHistory&) = delete;

            /** Check if history has any values. */
            virtual bool empty() const = 0;
            
            /** Return latest date in history, i.e. the last date for which a value was appended to history.
             * @throws std::logic_error if history is empty
             */
            virtual Date last_date() const = 0;
            
            /** Return last date on or before asof
             *	      @throw std::logic_error if history is empty.
             @throw std::out_of_range If no date <= asof
             */
            virtual Date last_date(Date asof) const = 0;
            
            /** Return first date in history.
             * @throws std::logic_error if history is empty
             */
            virtual Date first_date() const = 0;
            
            /** Return value as of given date, converted to double_t.
             * @param[in] asof As of date (valid date)
             * @return Value as of given date converted to double_t, or NaN if there is no value on this date.
             * @throw std::logic_error If history value type cannot be converted to double_t.
             */
            virtual double_t as_double(Date asof) const = 0;
            
            /** Return value as of given date, converted to int_t.
             * @param[in] asof As of date (valid date)
             * @return Value as of given date converted to int_t, if available.
             * @throw std::logic_error If history value type cannot be converted to int_t
             * @throw std::runtime_error If there is no value on this date.
             */
            virtual int_t as_int(Date asof) const = 0;
            
            /** Last value as double_t.
             * @return Last value converted to double_t.
             * @throw std::logic_error If history value type cannot be converted to double_t or history is empty.
             */
            virtual double_t last_as_double() const = 0;
            
            /** Last value as int_t.
             * @return Last value converted to int_t.
             * @throw std::logic_error If history value type cannot be converted to int_t or history is empty.
             */
            virtual int_t last_as_int() const = 0;
            
            /** Last value on or before given date, as double_t.
             * @param[in] asof As of date (valid date)
             * @return Last value converted to double_t
             * @throw std::logic_error If history value type cannot be converted to double_t.
             @throw std::out_of_range If no date <= asof
             */
            virtual double_t last_as_double(Date asof) const = 0;
            
            /** Last value on or before given date, as int_t.
             * @param[in] asof As of date (valid date)
             * @return Last value converted to int_t.
             * @throw std::logic_error If history value type cannot be converted to int_t.
             * @throw std::out_of_range If there is no value on or before the asof date.
             */
            virtual int_t last_as_int(Date asof) const = 0;

            /** Number of events stored */
            virtual index_t size() const = 0;

            /** Get idx-th event date
              @throw std::out_of_range If idx >= size()
            */
            virtual Date date(index_t idx) const = 0;

            /** Get idx-th event value as double_t
              @throw std::out_of_range If idx >= size()
             */
            virtual double_t as_double(index_t idx) const = 0;

            /** Get idx-th event value as int_t
              @throw std::out_of_range If idx >= size()
             */
            virtual int_t as_int(index_t idx) const = 0;

            /** Last index on or before asof.
              @throw std::out_of_range If there is no event on or before asof.
            */
            virtual index_t last_index(Date asof) const = 0;

            /** First index on or after asof.
              @throw std::out_of_range If there is no event on or after asof.
            */
            virtual index_t first_index(Date asof) const = 0;

			/** Convert to a pure data object */
			virtual HistoryData to_data() const = 0;

			/** History name, for debugging purposes */
			virtual const std::string& name() const = 0;
        };
    }
}

#endif // __AVERISERA_MS_IMMUTABLE_HISTORY_H
