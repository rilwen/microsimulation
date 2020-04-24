/*
 * (C) Averisera Ltd 2015
 */
#include "config_interpreter.hpp"
#include "core/object_value.hpp"
#include <cassert>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
    namespace microsim {
        std::string ConfigInterpreter::trim_whitespace(const std::string& expr) {
            std::string trimmed(expr);
            boost::algorithm::trim(trimmed);
            return trimmed;
        }
        
        int ConfigInterpreter::find_at_level(const std::string& expr, const char what, const unsigned int required_level) {
            int level = 0;
            for (auto it = expr.begin(); it < expr.end(); ++it) {
                const char c = *it;
                switch (c) {
                    case '(':
                        if ('(' == what && level == static_cast<int>(required_level)) {
                            return static_cast<int>(it - expr.begin());
                        }
                        ++level;
                        break;
                    case ')':
                        --level;
                        if (')' == what && level == static_cast<int>(required_level)) {
                            return static_cast<int>(it - expr.begin());
                        }
                        break;
                    default:
                        if (c == what && level == static_cast<int>(required_level)) {
                            return static_cast<int>(it - expr.begin());
                        }
                }
                if (level < 0) {
                    throw std::runtime_error(std::string("ConfigInterpreter: surplus closing parentheses: ") + expr);
                }
            }
            if (level != 0) {
                assert(level > 0);
                throw std::runtime_error("ConfigInterpreter: surplus opening parentheses");
            }
            return -1;
        }
        
        std::pair<std::string, std::vector<std::string>> ConfigInterpreter::split_expr(const std::string& expr, bool& is_call) {
            std::string trimmed(trim_whitespace(expr));
            if (trimmed.empty()) {
                throw std::runtime_error("ConfigInterpreter: splitting empty expression");
            }
            const int left_idx = find_at_level(trimmed, '(', 0);
            const int right_idx = find_at_level(trimmed, ')', 0);
            if (left_idx > 0 && right_idx == static_cast<int>(trimmed.size()) - 1) {
                is_call = true;
                const std::string func(trim_whitespace(trimmed.substr(0, left_idx)));
                trimmed = trim_whitespace(trimmed.substr(left_idx + 1, right_idx - left_idx - 1));
                std::vector<std::string> args;
                while (!trimmed.empty()) {
                    const int idx = find_at_level(trimmed, ',', 0);
                    if (idx >= 0) {						
                        const std::string arg(trim_whitespace(trimmed.substr(0, idx)));
                        if (arg.empty()) {
                            throw std::runtime_error("ConfigInterpreter: empty argument");
                        } else {
                            args.push_back(arg);
                        }
                        trimmed = trim_whitespace(trimmed.substr(idx + 1));
                        if (trimmed.empty()) {
                            throw std::runtime_error("ConfigInterpreter: empty argument");
                        }
                    } else {
                        // just 1 argument
                        args.push_back(trimmed);
                        break;
                    }
                }
                return std::make_pair(func, args);
            } else {
                is_call = false;
                if (!trimmed.empty()) {
                    return std::make_pair(trimmed, std::vector<std::string>());
                } else {
                    throw std::runtime_error("ConfigInterpreter: empty expression");
                }
            }
        }
        
        ObjectValue ConfigInterpreter::parse_token(const std::string& token) {
            std::string trimmed(trim_whitespace(token));
            if (trimmed.empty()) {
                throw std::runtime_error("ConfigInterpreter: empty token");
            }
            if ("false" == trimmed) {
                return ObjectValue(false);
            } else if ("true" == trimmed) {
                return ObjectValue(true);
            } else if ('"' == trimmed[0]) {
                if (trimmed.size() < 2 || '"' != trimmed.back()) {
                    throw std::runtime_error("ConfigInterpreter: malformed string");
                }
                // it should be a string
                bool escaped = false;
                const auto end = trimmed.end() - 1;
                std::string res;
                for (auto it = trimmed.begin() + 1; it != end; ++it) {
                    const char c = *it;
                    if (escaped) {
                        escaped = false;
                        if ('\\' == c || '"' == c) {
                            res += c;
                        } else {
                            throw std::runtime_error("ConfigInterpreter: unsupported escaping");
                        }
                    } else {
                        if ('\\' == c) {
                            escaped = true;
                        } else if ('"' == c) {
                            throw std::runtime_error("ConfigInterpreter: non-escaped special character");
                        } else {
                            res += c;
                        }
                    }
                }
                if (escaped) {
                    throw std::runtime_error("ConfigInterpreter: hanging escape character");
                }
                return ObjectValue(res);
            } else {
                try {
                    const double val = boost::lexical_cast<double>(trimmed);
                    return ObjectValue(val);
                } catch (boost::bad_lexical_cast& e) {
                    throw std::runtime_error(std::string("ConfigInterpreter: ") + e.what());
                }
            }
        }
        
		ConfigInterpreter::value_type ConfigInterpreter::interpret_expression(const std::string& expr) const {
            bool is_call;
            const auto split = split_expr(expr, is_call);
            if (is_call) {
                argvec_type parsed_args(split.second.size());
                std::transform(split.second.begin(), split.second.end(), parsed_args.begin(), [this](const std::string& arg){ return interpret_expression(arg); });
                return interpret_call(split.first, parsed_args);
            } else {
                return parse_token(split.first);
            }
        }
        
        ConfigInterpreter::value_type ConfigInterpreter::interpret_call(const std::string& func, const ConfigInterpreter::argvec_type& args) const {
            validate_function_name(func);
            const auto fit = _functions.find(func);
            if (fit != _functions.end()) {
                return fit->second(args);
            } else {
                throw std::runtime_error(std::string("ConfigInterpreter: unknown function: ") + func);
            }
        }        
        
        void ConfigInterpreter::validate_function_name(const std::string& func) {
            for (auto it = func.begin(); it != func.end(); ++it) {
                const char c = *it;
                if (! ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (it > func.begin() && c >= '0' && c <= '9'))) {
                    throw std::runtime_error(std::string("ConfigInterpreter: malformed function name: ") + func);
                }
            }
        }
    }
}
