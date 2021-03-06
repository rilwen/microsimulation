// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_PRED_TRUE_H
#define __AVERISERA_MS_PRED_TRUE_H

#include "../predicate.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        /** Predicate which always returns true */
        template <class T> class PredTrue: public Predicate<T> {
        public:
            bool select(const T&, const Contexts&) const override {
                return true;
            }

            bool select_out_of_context(const T&) const override {
                return true;
            }
            
            bool always_true() const override {
                return true;
            }

            bool always_true_out_of_context() const override {
                return true;
            }

            Predicate<T>* clone() const override {
                return new PredTrue<T>();
            }

            std::shared_ptr<const Predicate<T> > sum(const Predicate<T>&) const override {
                // True || x == True
                return std::make_shared<PredTrue<T>>();
            }

            std::shared_ptr<const Predicate<T> > sum(std::shared_ptr<const Predicate<T> >) const override {
                // True || x == True
                return std::make_shared<PredTrue<T>>();
            }

            std::shared_ptr<const Predicate<T> > product(std::shared_ptr<const Predicate<T> > other) const override {
                if (other) {
                    // True && x == x
                    return other;
                } else {
                    throw std::domain_error("PredTrue: null other");
                }
            }

			void print(std::ostream& os) const override {
				os << "True";
			}
        };
    }
}

#endif // __AVERISERA_MS_PRED_TRUE_H
