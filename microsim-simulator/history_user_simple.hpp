#ifndef __AVERISERA_MICROSIM_HISTORY_USER_SIMPLE_HPP
#define  __AVERISERA_MICROSIM_HISTORY_USER_SIMPLE_HPP

#include "history_user.hpp"

namespace averisera {
    namespace microsim {
        /** Simple HistoryUser implementation which stores a vector of requirements */
        template <class T> class HistoryUserSimple: public HistoryUser<T> {
        public:
            using typename HistoryUser<T>::use_req_t;
            using typename HistoryUser<T>::use_reqvec_t;

            /** Create a single history user */
            HistoryUserSimple(const std::string& name)
                : HistoryUserSimple(use_reqvec_t(1, name)) {
            }

            /** Create a user for multiple histories -- most general case */
            HistoryUserSimple(use_reqvec_t&& requirements) noexcept
                : _requirements(std::move(requirements)) {
            }

            /** Insert names
            */
            template <class SI> HistoryUserSimple(SI names_begin, const SI names_end) {
                while (names_begin != names_end) {
                    _requirements.push_back(use_req_t(*names_begin));
                    ++names_begin;
                }
            }

			/** Move constructor */
			HistoryUserSimple(HistoryUserSimple&& other) noexcept
				: _requirements(std::move(other._requirements)) {
				other._requirements.clear();
			}

            const use_reqvec_t& user_requirements() const override {
                return _requirements;
            }
        private:
            use_reqvec_t _requirements;
        };
    }
}

#endif //  __AVERISERA_MICROSIM_HISTORY_USER_SIMPLE_HPP
