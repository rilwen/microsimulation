/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_HISTORY_SPARSE_H
#define __AVERISERA_MS_HISTORY_SPARSE_H

#include "../history.hpp"
#include <memory>

namespace averisera {
    namespace microsim {
        /*! \brief History implementation which uses sparse storage and stores only values when they are changed. It assumes that between the changes the value is known and constant.
         * 
         *	  When appending a double (or int) value to HistorySparse, it is appended to implementation only if it is different than the vale
         *	  returns by last_as_double() (or last_as_int()), or implementation is empty.
         
         Index-based accesses work just like in the normal history, i.e. we iterate over the value-change events.
         
         
         */
        class HistorySparse: public History {
        public:
            /*! Construct a new HistorySparse.
             * 
             * Takes ownership of the provided pointer to implementation.
             *
             * \param impl Implementation of History which stores changing values. Not null.
             * \throw std::domain_error If impl is null.
             */
            HistorySparse(std::unique_ptr<History>&& impl);
            
            bool empty() const {
                return _impl->empty();
            }
            
            Date first_date() const {
                return _impl->first_date();
            }
            
            Date last_date() const;
            
            Date last_date(Date asof) const;	    
            
            double_t as_double(Date date) const;
            
            int_t as_int(Date date) const;
            
            double_t last_as_double() const {
                return _impl->last_as_double();
            }
            
            int_t last_as_int() const {
                return _impl->last_as_int();
            }
            
            double_t last_as_double(Date date) const {
                return _impl->last_as_double(date);
            }
            
            int_t last_as_int(Date date) const {
                return _impl->last_as_int(date);
            }
            
            void append(Date date, double_t value) override;
            
            void append(Date date, int_t value) override;

            void correct(double_t value) override;

            index_t size() const override {
                return _impl->size();
            }

            Date date(index_t idx) const override {
                return _impl->date(idx);
            }

            double_t as_double(index_t idx) const override {
                return _impl->as_double(idx);
            }

            int_t as_int(index_t idx) const override {
                return _impl->as_int(idx);
            }

            index_t last_index(Date asof) const override {
                return _impl->last_index(asof);
            }

            index_t first_index(Date asof) const override {
                return _impl->first_index(asof);
            }

            void print(std::ostream& os) const override {
                os << "sparse " << *_impl;
            }

            std::unique_ptr<History> clone() const override;

			HistoryData to_data() const override;

			const std::string& name() const override {
				return _impl->name();
			}
        private:
            template <class T> void do_append(Date date, T value);            
            
            std::unique_ptr<History> _impl;
            Date _last_date;
        };
    }
}

#endif
