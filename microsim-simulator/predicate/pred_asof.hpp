#pragma once
#include "../contexts.hpp"
#include "../predicate.hpp"
#include "core/dates.hpp"
#include "core/preconditions.hpp"
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		/*! Selects an object if the asof date is in [begin, end) range */
		template <class T> class PredAsof: public Predicate<T> {
		public:
			/*! \throw std::domain_error if !(begin <= end) */
			PredAsof(Date begin, Date end)
				: begin_(begin), end_(end) {
				check_that(begin <= end, "PredAsof: !(begin <= end)");				
			}

			bool active(Date date) const override {
				return date >= begin_ && date < end_;
			}

			bool select(const T&, const Contexts& contexts) const override {
				return active(contexts.asof());
			}

			bool select_out_of_context(const T&) const override {
				return true;
			}

			bool always_true_out_of_context() const override {
				return true;
			}

			PredAsof* clone() const override {
				return new PredAsof(begin_, end_);
			}

			void print(std::ostream& os) const override {
				os << "Asof(" << begin_ << ", " << end_ << ")";
			}
		private:
			Date begin_;
			Date end_;
			std::string str_;
		};
	}
}
