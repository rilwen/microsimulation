/*
 * (C) Averisera Ltd 2015
 */
#include "config_interpreter_core.hpp"
#include "config_interpreter.hpp"
#include "core/object_value.hpp"
#include <cmath>

namespace averisera {
    namespace microsim {
        namespace ConfigInterpreterCore {

			namespace {
				ConfigInterpreter::value_type abs_(const std::vector<ObjectValue>& v) {
					const auto& x = v.at(0);
					switch (x.type()) {
					case ObjectValue::Type::DOUBLE:
						return ConfigInterpreter::value_type(std::abs(x.as_double()));
					case ObjectValue::Type::INT:
						return ConfigInterpreter::value_type(std::abs(x.as_int()));
					default:
						throw std::runtime_error("Abs: unsupported object type");
					}
				}

				static ConfigInterpreter::value_type vec_(const ConfigInterpreter::argvec_type& v) {
					return ConfigInterpreter::value_type(v);
				}
			}

            void import(ConfigInterpreter& interpreter) {
				interpreter.add_function("Abs", abs_);
                interpreter.add_function("Vec", vec_);
            }
        }
    }
}
