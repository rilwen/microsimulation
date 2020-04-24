#ifndef __AVERISERA_MS_PRED_VARIABLE_RANGE_H
#define __AVERISERA_MS_PRED_VARIABLE_RANGE_H

//#include "../actor.h"
#include "../predicate.hpp"
#include <string>
//#include <type_traits>


namespace averisera {
    namespace microsim {
        /*! Predicate selecting Actor implementations with a variable within a range. 
        \tparam T Derived from Actor 
        \tparam V Value class (double or int) */
        template <class T, class V = double> class PredVariableRange : public Predicate<T> {
            //static_assert(std::is_base_of<Actor, T>::value, "T must be derived from Actor");
        public:
            /*!
            \param variable Variable name
            \param min Lower end of the range (inclusive)
            \param max Upper end of the range (inclusive)
            \param accept_missing Should objects with value unset be selected
            \throw std::domain_error If min > max or variable is empty
            */
            PredVariableRange(const std::string& variable, V min, V max, bool accept_missing = false);

            bool select(const T& obj, const Contexts& contexts) const override;			

            bool always_true() const override {
                return _always_true;
            }

            PredVariableRange<T, V>* clone() const override {
                return new PredVariableRange<T, V>(_variable, _min, _max, _accept_missing);
            }

            bool select_out_of_context(const T& obj) const override {
                return true;
            }

            bool always_true_out_of_context() const override {
                return true;
            }

			void print(std::ostream& os) const override;
        private:
            std::string _variable;
            V _min;
            V _max;
            bool _accept_missing;
            bool _always_true;
        };
    }
}

#endif // __AVERISERA_MS_PRED_VARIABLE_RANGE_H
