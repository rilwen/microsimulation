// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_GENERIC_DISTRIBUTION_ENUMERATED_H
#define __AVERISERA_GENERIC_DISTRIBUTION_ENUMERATED_H

#include "generic_distribution_integral.hpp"
#include "sorting.hpp"
#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <vector>

namespace averisera {
    /** Distribution of probability over enumerated, sorted elements. */
    template <class T> class GenericDistributionEnumerated: public GenericDistribution<T> {
    public:
        /** @throw std::domain_error If values.size() != probs.size(), or if values are not sorted and unique. */
        template <class V> GenericDistributionEnumerated(const std::vector<T>& values, const V& probs)
            : _values(values), _index_distr(0, probs) {
            if (!std::is_sorted(_values.begin(), _values.end())) {
                throw std::domain_error("GenericDistributionEnumerated: values are not sorted");
            }            
            if (std::adjacent_find(_values.begin(), _values.end()) != _values.end()) {
                throw std::domain_error("GenericDistributionEnumerated: values are not unique");
            }
            if (_values.size() != probs.size()) {
                throw std::domain_error("GenericDistributionEnumerated: vector size mismatch");
            }
        }

        /** @throw std::domain_error If values.size() != probs.size() */
        GenericDistributionEnumerated(std::vector<T>&& values, std::vector<double>&& probs)
            : _values(std::move(values)), _index_distr(0, std::move(probs)) {
            if (!std::is_sorted(_values.begin(), _values.end())) {
                cleanup(values, probs);
                throw std::domain_error("GenericDistributionEnumerated: values are not sorted");
            }            
            if (std::adjacent_find(_values.begin(), _values.end()) != _values.end()) {
                cleanup(values, probs);
                throw std::domain_error("GenericDistributionEnumerated: values are not unique");
            }
            if (_values.size() != _index_distr.size()) {
                cleanup(values, probs);
                throw std::domain_error("GenericDistributionEnumerated: vector size mismatch");
            }
        }

		/** Sort values and probs before calling the constructor */
		template <class V> static std::unique_ptr<GenericDistributionEnumerated> from_unsorted(const std::vector<T>& values, const V& probs) {
			const size_t n = values.size();
			std::vector<const T*> pointers(n);
			std::transform(values.begin(), values.end(), pointers.begin(), [](const T& t) { return &t; });
			Sorting::sort_pointers(pointers);
			std::vector<T> sorted_values(n);
			std::vector<double> sorted_probs(n);
			const T* const p0 = &values[0];
			auto sv_it = sorted_values.begin();
			auto sp_it = sorted_probs.begin();
			for (const T* const p : pointers) {
				*sv_it = *p;
				*sp_it = probs[std::distance(p0, p)];
				++sv_it;
				++sp_it;
			}
			assert(sv_it == sorted_values.end());
			assert(sp_it == sorted_probs.end());
			return std::make_unique<GenericDistributionEnumerated>(std::move(sorted_values), std::move(sorted_probs));
		}

        T icdf_generic(double p) const override {
            return _values[_index_distr.icdf_generic(p)];
        }

        T random(RNG& rng) const override {
            return _values[_index_distr.random(rng)];
        }

        double range_prob2(T x1, T x2) const override {
            if (x1 < x2) {
                return cdf2(x2) - cdf2(x1);
            } else {
                return 0.;
            }
        }

        /** P(X < x) */
        double cdf2(T x) const;

        T lower_bound() const override {
            return _values.front();
        }

        T upper_bound() const override {
            return _values.back();
        }

        GenericDistributionEnumerated<T>* conditional(T left, T right) const override;

        unsigned int size() const {
            return _index_distr.size();
        }
    private:
        void cleanup(std::vector<T>& values, std::vector<double>& probs) {
            // put them back!
            values = std::move(_values);
            probs = _index_distr.probs();
        }

		std::vector<T> _values;
        GenericDistributionIntegral<ptrdiff_t> _index_distr;
    };

    template <class T> double GenericDistributionEnumerated<T>::cdf2(T x) const {
        if (x <= _values.front()) {
            return 0.;
        } else if (x > _values.back()) {
            return 1.;
        } else {
            // Look for the last value less than x
            const auto it = std::upper_bound(_values.rbegin(), _values.rend(), x, std::greater<T>());
            assert(it != _values.rend());
            assert(*it < x);
            return _index_distr.cdf2(_values.size() - std::distance(_values.rbegin(), it));
        }
    }

    template <class T> GenericDistributionEnumerated<T>* GenericDistributionEnumerated<T>::conditional(const T left, const T right) const {
        // find what indices to include
        ptrdiff_t i0, i1;
        if (left <= _values.front()) {
            i0 = 0;
        } else {
            // find the index of first value which is not smaller than left
            const auto it = std::lower_bound(_values.begin(), _values.end(), left);
            i0 = it - _values.begin();
        }
        if (right > _values.back()) {
            i1 = _values.size() - 1;
        } else {
            // find the index of the last value which is strictly less than right
            const auto it = std::upper_bound(_values.rbegin(), _values.rend(), right, std::greater<T>());
            i1 = _values.size() - 1 - std::distance(_values.rbegin(), it);
        }
        if (i0 > i1) {
            throw std::runtime_error("GenericDistributionEnumerated: zero probability in the selected region");
        } else {
            std::vector<T> sel_values(_values.begin() + i0, _values.begin() + (i1 + 1));
            std::vector<double> sel_probs(_index_distr.probs().begin() + i0, _index_distr.probs().begin() + (i1 + 1));
            const double sump = std::accumulate(sel_probs.begin(), sel_probs.end(), 0.0);
            std::for_each(sel_probs.begin(), sel_probs.end(), [sump](double& p) { p /= sump; });
            return new GenericDistributionEnumerated<T>(std::move(sel_values), std::move(sel_probs));
        }
    }
}

#endif // __AVERISERA_GENERIC_DISTRIBUTION_ENUMERATED_H
