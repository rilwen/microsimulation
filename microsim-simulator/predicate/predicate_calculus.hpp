#ifndef __AVERISERA_MICROSIM_PREDICATE_CALCULUS_H
#define __AVERISERA_MICROSIM_PREDICATE_CALCULUS_H

/** Defines both PredAnd and PredOr */

#include "core/preconditions.hpp"
#include <algorithm>
#include <memory>
#include <vector>
#include <stdexcept>
#include <initializer_list>


namespace averisera {
    namespace microsim {
        /** @brief Logical product of two predicates
		*/
		template <class T> class PredAnd: public Predicate < T > {
		public:
			/**
			@param[in] predicates list of predicates
			@throw std::domain_error If any predicate is null or predicates.empty()
			*/
			PredAnd(std::initializer_list<std::shared_ptr<const Predicate<T>>> predicates) 
				: _predicates(predicates), _always_true(is_always_true()), _always_true_out_of_context(is_always_true_out_of_context())				
			{
				check_that(predicates.size() > 0, "PredAnd: no predicates");
                check_all_not_null(predicates, "PredAnd: null predicates");
				require_alive_ = std::any_of(predicates.begin(), predicates.end(), [](const std::shared_ptr<const Predicate<T>>& p) {
					return p->selects_alive_only();
				});
			}

            /**
			@param[in] predicates vector of predicates
			@throw std::domain_error If any predicate is null
			*/
			PredAnd(const std::vector<std::shared_ptr<const Predicate<T>>>& predicates) 
				: _predicates(predicates), _always_true(is_always_true()), _always_true_out_of_context(is_always_true_out_of_context())
			{
				check_that(!predicates.empty(), "PredAnd: no predicates");
				check_all_not_null(predicates, "PredAnd: null predicates");
				require_alive_ = std::any_of(predicates.begin(), predicates.end(), [](const std::shared_ptr<const Predicate<T>>& p) {
					return p->selects_alive_only();
				});
			}

            /** Move constructor
              @param[in] predicates List of predicates
              @throw std::domain_error If any predicate is null
			*/
            PredAnd(std::vector<std::shared_ptr<const Predicate<T>>>&& predicates) {
				check_that(!predicates.empty(), "PredAnd: no predicates");
				check_all_not_null(predicates, "PredAnd: null predicates");
                _predicates = std::move(predicates);
                _always_true = is_always_true();
                _always_true_out_of_context = is_always_true_out_of_context();
				require_alive_ = std::any_of(_predicates.begin(), _predicates.end(), [](const std::shared_ptr<const Predicate<T>>& p) {
					return p->selects_alive_only();
				});
            }

			bool active(Date date) const override {
				return std::all_of(_predicates.begin(), _predicates.end(), [date](const std::shared_ptr<const Predicate<T>>& pred) {
					return pred->active(date);
				});
			}

			bool select(const T& obj, const Contexts& contexts) const override {
				return std::all_of(_predicates.begin(), _predicates.end(), [&obj, &contexts](const std::shared_ptr<const Predicate<T>>& pred) {
                        return pred->select(obj, contexts);
                    });
			}

			bool select_alive(const T& obj, const Contexts& contexts) const override {
				return std::all_of(_predicates.begin(), _predicates.end(), [&obj, &contexts](const std::shared_ptr<const Predicate<T>>& pred) {
					return pred->select_alive(obj, contexts);
				});
			}

            bool select_out_of_context(const T& obj) const override {
				return std::all_of(_predicates.begin(), _predicates.end(), [&obj](const std::shared_ptr<const Predicate<T>>& pred) {
                        return pred->select_out_of_context(obj);
                    });
			}

            bool always_true() const override {
                return _always_true;
            }

            bool always_true_out_of_context() const override {
                return _always_true_out_of_context;
            }

            Predicate<T>* clone() const override {
                return new PredAnd<T>(*this);
            }

            std::shared_ptr<const Predicate<T> > product(std::shared_ptr<const Predicate<T> > other) const override;

            // for testing
            size_t size() const {
                return _predicates.size();
            }

			void print(std::ostream& os) const override {
				os << "And(";
				auto it = _predicates.begin();
				assert(it != _predicates.end());
				(*it)->print(os);
				++it;
				for (; it != _predicates.end(); ++it) {
					os << ", ";
					(*it)->print(os);
				}
				assert(it == _predicates.end());
				os << ")";
			}

			bool selects_alive_only() const override {
				return require_alive_;
			}
		private:
			std::vector<std::shared_ptr<const Predicate<T>>> _predicates;
            bool _always_true;
            bool _always_true_out_of_context;
			bool require_alive_;

            bool is_always_true() const {
                return std::all_of(_predicates.begin(), _predicates.end(), [](const std::shared_ptr<const Predicate<T>>& pred) {
                        return pred->always_true();
                    });
            }

            bool is_always_true_out_of_context() const {
                return std::all_of(_predicates.begin(), _predicates.end(), [](const std::shared_ptr<const Predicate<T>>& pred) {
                        return pred->always_true_out_of_context();
                    });
            }
		};

        /** @brief Logical sum of two predicates
         */
		template <class T> class PredOr : public Predicate<T> {
		public:
            /**
			@param[in] predicates list of predicates
			@throw std::domain_error If any predicate is null
			*/
			PredOr(std::initializer_list<std::shared_ptr<const Predicate<T>>> predicates) 
				: _predicates(predicates), _always_true(is_always_true()), _always_true_out_of_context(is_always_true_out_of_context())
			{
				check_that(predicates.size() > 0, "PredOr: no predicates");
				check_all_not_null(predicates, "PredOr: null predicates");
				require_alive_ = std::all_of(predicates.begin(), predicates.end(), [](const std::shared_ptr<const Predicate<T>>& p) {
					return p->selects_alive_only();
				});
			}

			/**
              @param[in] predicates List of predicates
              @throw std::domain_error If any predicate is null
			*/
			PredOr(const std::vector<std::shared_ptr<const Predicate<T>>>& predicates)
				: _predicates(predicates), _always_true(is_always_true()), _always_true_out_of_context(is_always_true_out_of_context())
                {
					check_that(!predicates.empty(), "PredOr: no predicates");
					check_all_not_null(predicates, "PredOr: null predicates");
					require_alive_ = std::all_of(predicates.begin(), predicates.end(), [](const std::shared_ptr<const Predicate<T>>& p) {
						return p->selects_alive_only();
					});
                }

            /** Move constructor
              @param[in] predicates List of predicates
              @throw std::domain_error If any predicate is null
			*/
            PredOr(std::vector<std::shared_ptr<const Predicate<T>>>&& predicates) {
				check_that(!predicates.empty(), "PredOr: no predicates");
				check_all_not_null(predicates, "PredOr: null predicates");
                _predicates = std::move(predicates);
                _always_true = is_always_true();
                _always_true_out_of_context = is_always_true_out_of_context();
				require_alive_ = std::all_of(_predicates.begin(), _predicates.end(), [](const std::shared_ptr<const Predicate<T>>& p) {
					return p->selects_alive_only();
				});
            }
            
			bool active(Date date) const override {
				return std::any_of(_predicates.begin(), _predicates.end(), [date](const std::shared_ptr<const Predicate<T>>& pred) {
					return pred->active(date);
				});
			}

			bool select(const T& obj, const Contexts& contexts) const override {
                return std::any_of(_predicates.begin(), _predicates.end(), [&obj, &contexts](const std::shared_ptr<const Predicate<T>>& pred) {
                        return pred->select(obj, contexts);
                    });
			}

			bool select_alive(const T& obj, const Contexts& contexts) const override {
				return std::any_of(_predicates.begin(), _predicates.end(), [&obj, &contexts](const std::shared_ptr<const Predicate<T>>& pred) {
					return pred->select_alive(obj, contexts);
				});
			}

            bool select_out_of_context(const T& obj) const override {
				return std::any_of(_predicates.begin(), _predicates.end(), [&obj](const std::shared_ptr<const Predicate<T>>& pred) {
                        return pred->select_out_of_context(obj);
                    });
			}

            bool always_true() const override {
                return _always_true;
            }

            bool always_true_out_of_context() const override {
                return _always_true_out_of_context;
            }

            PredOr<T>* clone() const override {
                return new PredOr<T>(*this);
            }

            std::shared_ptr<const Predicate<T> > sum(std::shared_ptr<const Predicate<T> > other) const override;

            // for testing
            size_t size() const {
                return _predicates.size();
            }

			void print(std::ostream& os) const override {
				os << "Or(";
				auto it = _predicates.begin();
				assert(it != _predicates.end());
				(*it)->print(os);
				++it;
				for (; it != _predicates.end(); ++it) {
					os << ", ";
					(*it)->print(os);
				}
				assert(it == _predicates.end());
				os << ")";
			}

			bool selects_alive_only() const override {
				return require_alive_;
			}
		private:
            PredOr(const PredOr<T>& other) = default;

            bool is_always_true() const {
                return std::any_of(_predicates.begin(), _predicates.end(), [](const std::shared_ptr<const Predicate<T>>& pred) {
                        return pred->always_true();
                    });
            }

            bool is_always_true_out_of_context() const {
                return std::any_of(_predicates.begin(), _predicates.end(), [](const std::shared_ptr<const Predicate<T>>& pred) {
                        return pred->always_true_out_of_context();
                    });
            }

            std::vector<std::shared_ptr<const Predicate<T>>> _predicates;
            bool _always_true;
            bool _always_true_out_of_context;
			bool require_alive_;
		};

        /** @brief Logical negation of a predicate
		*/
		template <class T> class PredNot : public Predicate < T > {
		public:
			/**
			@param[in] pred Predicate
			@throw std::domain_error If param is null
			*/
			PredNot(std::shared_ptr<const Predicate<T>> pred)
				: _pred(pred)
			{
				if (!pred) {
					throw std::domain_error("PredNot: Input predicate is null");
				}
			}

			bool select(const T& obj, const Contexts& contexts) const override {
				return !_pred.get()->select(obj, contexts);
			}

			bool select_alive(const T& obj, const Contexts& contexts) const override {
				return !_pred.get()->select_alive(obj, contexts);
			}

            bool select_out_of_context(const T& obj) const override {
				return !_pred.get()->select_out_of_context(obj);
			}

			bool active(Date date) const override {
				return !_pred.get()->active(date);
			}

            Predicate<T>* clone() const override {
                return new PredNot<T>(_pred);
            }

            std::shared_ptr<const Predicate<T>> negate() const override {
                // not(not x) == x
                return _pred;
            }

			void print(std::ostream& os) const override {
				os << "Not(";
				_pred->print(os);
				os << ")";
			}

			bool selects_alive_only() const override {
				return false;
			}
		private:
			std::shared_ptr<const Predicate<T>> _pred;
		};

        // Definitions of some functions

        template <class T> std::shared_ptr<const Predicate<T> > PredOr<T>::sum(std::shared_ptr<const Predicate<T> > other) const {
            std::vector<std::shared_ptr<const Predicate<T>>> new_preds(_predicates);
            new_preds.push_back(other);
            return std::make_shared<PredOr<T>>(std::move(new_preds));
        }

        template <class T> std::shared_ptr<const Predicate<T> > PredAnd<T>::product(std::shared_ptr<const Predicate<T> > other) const {
            std::vector<std::shared_ptr<const Predicate<T>>> new_preds(_predicates);
            new_preds.push_back(other);
            return std::make_shared<PredAnd<T>>(std::move(new_preds));
        }
    }
}

#endif // __AVERISERA_MICROSIM_PREDICATE_CALCULUS_H
