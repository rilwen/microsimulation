// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_HISTORY_GENERATOR_SIMPLE_HPP
#define __AVERISERA_MICROSIM_HISTORY_GENERATOR_SIMPLE_HPP

#include "history_generator.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        /** Simple HistoryGenerator implementation which stores a vector of requirements */
        template <class T> class HistoryGeneratorSimple: public HistoryGenerator<T> {
        public:
            using typename HistoryGenerator<T>::factory_t;
            using typename HistoryGenerator<T>::req_t;
            using typename HistoryGenerator<T>::reqvec_t;

            /** Create a dense double history generator 
              @param predicate Cannot be null
              @throw std::domain_error If  predicate is null.
             */
            HistoryGeneratorSimple(const std::string& name, std::shared_ptr<const Predicate<T> > predicate)
                : HistoryGeneratorSimple(reqvec_t(1, req_t(name, HistoryFactory::DENSE<double>(), predicate))) {
                if (!predicate) {
                    throw std::domain_error("HistoryGeneratorSimple: null predicate");
                }                
            }

            /** Create a single History generator
              @param predicate Cannot be null
              @throw std::domain_error If  predicate is null.
             */
            HistoryGeneratorSimple(const std::string& name, factory_t factory, std::shared_ptr<const Predicate<T> > predicate)
                : HistoryGeneratorSimple(reqvec_t(1, req_t(name, factory, predicate))) {
                if (!predicate) {
                    throw std::domain_error("HistoryGeneratorSimple: null predicate");
                }
            }

            /** Create a generator for multiple histories -- most general case
              Does no input checks.
             */
            HistoryGeneratorSimple(reqvec_t&& requirements) noexcept
                : _requirements(std::move(requirements)) {
            }

			HistoryGeneratorSimple(HistoryGeneratorSimple&& other) noexcept
				: _requirements(std::move(other._requirements)) {
				other._requirements.clear();
			}

            /** Provided a range of names and history factories and a single predicate, construct the requirements sharing the predicate.
              @param predicate Cannot be null
              @throw std::domain_error If ranges have unequal size or predicate is null.
            */
            template <class SI, class FI> HistoryGeneratorSimple(SI names_begin, const SI names_end, FI factories_begin, const FI factories_end, std::shared_ptr<const Predicate<T> > predicate) {
                if (!predicate) {
                    throw std::domain_error("HistoryGeneratorSimple: null predicate");
                }
                while ((names_begin != names_end) && (factories_begin != factories_end)) {
                    _requirements.push_back(req_t(*names_begin, *factories_begin, predicate));
                    ++names_begin;
                    ++factories_begin;
                }
                if (names_begin != names_end || factories_begin != factories_end) {
                    throw std::domain_error("HistoryGeneratorSimple: unequal names and factories range sizes");
                }
            }

            const reqvec_t& requirements() const override {
                return _requirements;
            }
        private:
            reqvec_t _requirements;
        };
    }
}

#endif // __AVERISERA_MICROSIM_HISTORY_GENERATOR_SIMPLE_HPP
