#ifndef __AVERISERA_MICROSIM_OBSERVED_QUANTITY_H
#define __AVERISERA_MICROSIM_OBSERVED_QUANTITY_H

#include "../contexts.hpp"
#include "../immutable_history.hpp"
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>

namespace averisera {
    namespace microsim {
        /** Defines observed quantity for ObserverStats 
		Return NaN if data are missing.
		*/
        template <class T> class ObservedQuantity {
        public:
            /** @throw std::domain_error If fun is null */
            ObservedQuantity(const std::string& name, std::function<double(const T&, const Contexts&)> fun)
                : _name(name), _fun(fun) {
                if (!fun) {
                    throw std::domain_error("ObservedQuantity: null function");
                }
            }
            ObservedQuantity(const ObservedQuantity&) = default;
            ObservedQuantity& operator=(const ObservedQuantity&) = default;

            /** Move constructor */
            ObservedQuantity(ObservedQuantity&& other)
                : _name(std::move(other._name)), _fun(std::move(other._fun)) {
            }

            const std::string& name() const {
                return _name;
            }

            double operator()(const T& obj, const Contexts& ctx) const {
                return _fun(obj, ctx);
            }

            /** Factory method which creates an ObservedQuantity object which reads the last value of a variable
              with given name from its History, as double */
            static ObservedQuantity<T> last_as_double(const std::string& variable_name ) {
                return std::move(ObservedQuantity<T>(variable_name, std::function<double(const T&, const Contexts&)>([&variable_name](const T& obj, const Contexts& ctx) -> double {
					const ImmutableContext& imm_ctx = ctx.immutable_ctx();
					if (obj.has_history(imm_ctx, variable_name)) {
						const ImmutableHistory& history = obj.history(imm_ctx, variable_name);
						if (!history.empty()) {
							return history.last_as_double(ctx.asof());
						}
					}
					return std::numeric_limits<double>::quiet_NaN();
				})));
            }

			/** Variable which is either True (1) or False (0) */
			template <class I> static ObservedQuantity<T> indicator_variable(const std::string& variable_name, I indicator_fun) {
				return std::move(ObservedQuantity<T>(variable_name, std::function<double(const T&, const Contexts&)>([indicator_fun](const T& obj, const Contexts& ctx) -> double {
					return indicator_fun(obj, ctx) ? 1.0 : 0.0;
				})));
			}
        private:
            std::string _name;
            std::function<double(const T&, const Contexts&)> _fun;            
        };
    }
}

#endif // __AVERISERA_MICROSIM_OBSERVED_QUANTITY_H
