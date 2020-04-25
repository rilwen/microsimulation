/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_CONFIG_INTERPETER_H
#define __AVERISERA_MS_CONFIG_INTERPETER_H

#include <functional>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>

namespace averisera {
    class ObjectValue;
    
    namespace microsim {
        
        /** @brief Interprets config expressions
         * 
         * String constants are enclosed in double parenthes. Parenthes inside them must be escaped with \.
         * \ itself must be escaped. No other escaping supported.
         */
        class ConfigInterpreter {
        public:
            /** Trim whitespace from beginning and end of expression */
            static std::string trim_whitespace(const std::string& expr);
            
            /** Find a given character in a string at given parenthesis nesting level.
             * 
             \ param[in] expr Stri*ng to search
             @param[in] what Character sought
             @param[in] required_level Desired nesting level
             @return -1 if not found, index of the character if found
             @throw std::runtime_error If expr has unbalanced parentheses
             */
            static int find_at_level(const std::string& expr, char what, unsigned int required_level);
            
            /** Split a call expression of the form
             * 
             function(arg1,arg2,a*rg3)				(1)
             
             into (function, [arg1,arg2,arg2])
             
             If string is of the form
             
             func()								(2)
             
             return (expression, []). In both cases (1) and (2) set is_call to true.
             
             If string is of the form
             
             constant                                       (3)
             
             return (constant,[]) and set is_call to false.
             
             @param[in] expr String to be split
             @param[out] is_call Set to true if expr is a call expression, false if it is a constant.
             @throw std::runtime_error If expr is not of the form (1), (2) or (3).
             */
            static std::pair<std::string, std::vector<std::string>> split_expr(const std::string& expr, bool& is_call);

			typedef ObjectValue value_type;
            
            /** @brief Parse a single token.
             * 
             * @return A constant value, either a double number or a string or a bool ("true" or "false").
             * @throw std::runtime_error If not parsable as a constant value.
             */
            static value_type parse_token(const std::string& token);
            
            /** Interpret a general expression.
             * 
             * @throw std::runtime_error If expression incorrect.
             */
			value_type interpret_expression(const std::string& expr) const;
            
            typedef std::vector<value_type> argvec_type;
			// VS 2015 doesn't seem to handle std::function properly, so we use plain ol' function pointers
			typedef value_type(*func_type)(const argvec_type&);
            
            /** Add new function or replace previous with the same name. */
            void add_function(const std::string& name, func_type function) {
                validate_function_name(name);
                _functions[name] = function;
            }
        private:
            value_type interpret_call(const std::string& func, const argvec_type& args) const;
            static void validate_function_name(const std::string& func);
            
            std::unordered_map<std::string, func_type> _functions;
        };
    }
}

#endif // __AVERISERA_MS_CONFIG_INTERPETER_H
