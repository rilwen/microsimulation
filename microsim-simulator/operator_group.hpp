// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_OPERATOR_GROUP_H
#define __AVERISERA_MS_OPERATOR_GROUP_H

#include "operator.hpp"
#include "core/utils.hpp"
#include "dispatcher.hpp"
#include "history_generator_simple.hpp"
#include "predicate_factory.hpp"
#include <cassert>
#include <set>
#include <sstream>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        /** Operator composed of a group of operators and a Dispatcher which chooses the correct one for every argument.
         */
        template <class T> class OperatorGroup: public Operator<T>, public HistoryGeneratorSimple<T> {
        public:
            typedef Dispatcher<T, unsigned int> dispatcher_t;
            /** @param[in] operators Operators. 
             *  @param[in] dispatcher Dispatcher used to choose appropriate operator
             * @throw std::domain_error If operators is empty, any pointer in it is empty or operators have different values of provides(),
             * requires() or is_instantaneous(). Also if dispatcher is null.
             */
            OperatorGroup(const std::vector<std::shared_ptr<const Operator<T>>>& operators, std::shared_ptr<dispatcher_t> dispatcher);
            
            void apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const override;
            
            const Predicate<T>& predicate() const override {
                return *_dispatcher->predicate();
            }           

            /** Validate that operators are consistent. Used by other classes too. */
            static void validate(const std::vector<std::shared_ptr<const Operator<T>>>& operators);

			const std::string& name() const override {
				return name_;
			}
        private:
            static typename HistoryGenerator<T>::reqvec_t extract_history_requirements(const std::vector<std::shared_ptr<const Operator<T>>>& operators);
            std::vector<std::shared_ptr<const Operator<T>>> _operators;            
            std::shared_ptr<const dispatcher_t> _dispatcher;
			std::string name_;
        };
            
        
        template <class T> OperatorGroup<T>::OperatorGroup(const std::vector<std::shared_ptr<const Operator<T>>>& operators, std::shared_ptr<dispatcher_t> dispatcher) 
            : Operator<T>(Utils::pass_through(operators.front()->is_instantaneous(), [&operators](){validate(operators);}), operators.front()->provides(), operators.front()->requires()),
            HistoryGeneratorSimple<T>(std::move(extract_history_requirements(operators))),
            _operators(operators), _dispatcher(dispatcher) {
                if (!dispatcher) {
                    throw std::domain_error("OperatorGroup: dispatcher is null");
                }
				std::stringstream ss;
				assert(!_operators.empty());
				auto it = _operators.begin();
				ss << (*it)->name();
				++it;
				for (; it != _operators.end(); ++it) {
					ss << "_" << (*it)->name();
				}
				name_ = ss.str();
        }
        
        template <class T> void OperatorGroup<T>::validate(const std::vector<std::shared_ptr<const Operator<T>>>& operators) {
            if (operators.empty()) {
                throw std::domain_error("OperatorGroup: empty operators vector");
            }
            std::for_each(operators.begin(), operators.end(), [](const std::shared_ptr<const Operator<T>>& op) {
                if (!op) {
                    throw std::domain_error("OperatorGroup: null pointer");
                }
            });
            for (auto it = operators.begin() + 1; it != operators.end(); ++it) {
                if ((*it)->is_instantaneous() != operators.front()->is_instantaneous() ||
                    (*it)->requires() != operators.front()->requires() ||
                    (*it)->provides() != operators.front()->provides()) {
                    throw std::domain_error("OperatorGroup: inconsistent requirements");
                }
            }
        }
        
        template <class T> void OperatorGroup<T>::apply(const std::vector<std::shared_ptr<T>>& selected, const Contexts& contexts) const {
            
            std::vector<std::vector<std::shared_ptr<T>>> selected_groups(_operators.size());
            for (auto obj_it = selected.begin(); obj_it != selected.end(); ++obj_it) {
                const T& obj = **obj_it;
                const unsigned int idx = _dispatcher->dispatch(obj, contexts);
                assert(idx < selected_groups.size());
                selected_groups[idx].push_back(*obj_it);
            }
            auto sg_it = selected_groups.begin();
            for (auto it = _operators.begin(); it != _operators.end(); ++it, ++sg_it) {
                (*it)->apply(*sg_it, contexts);
            }
        }

        template <class T> typename HistoryGenerator<T>::reqvec_t OperatorGroup<T>::extract_history_requirements(const std::vector<std::shared_ptr<const Operator<T>>>& operators) {
            std::set<typename HistoryGenerator<T>::req_t> reqset;
            for (auto op: operators) {
                if (op) {
                    reqset.insert(op->requirements().begin(), op->requirements().end());
                } else {
                    throw std::domain_error("OperatorGroup: null operator");
                }
            }
            return typename HistoryGenerator<T>::reqvec_t(reqset.begin(), reqset.end());
        }
    }
}

#endif // __AVERISERA_MS_OPERATOR_GROUP_H
