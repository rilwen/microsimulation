// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_GENERIC_DISTRIBUTION_FROM_REAL_H
#define __AVERISERA_GENERIC_DISTRIBUTION_FROM_REAL_H

#include "distribution_conditional.hpp"
#include "generic_distribution.hpp"
#include "rng.hpp"
#include <memory>

namespace averisera {
    /** Generic distribution based on a real-valued distribution */
    template <class T> class GenericDistributionFromReal: public GenericDistribution<T> {
    public:
        /** @param cont_distr Real-valued distribution
          @throw std::domain_error If real_distr is null
        */
        GenericDistributionFromReal(std::shared_ptr<const Distribution> real_distr);

        T random(RNG& rng) const override {
            return from_double(_real_distr->draw(rng));
        } 

        double range_prob2(T x1, T x2) const override {
            return _real_distr->range_prob2(to_double(x1), to_double(x2));
        }

        T icdf_generic(double p) const override {
            return from_double(_real_distr->icdf(p));
        }

        T lower_bound() const override {
            return from_double(_real_distr->icdf(0));
        }

        T upper_bound() const override {
            return from_double(_real_distr->icdf(1));
        }
    protected:
        std::shared_ptr<Distribution> conditional_real_distr(T left, T right) const {
            const double a = to_double(left);
            const double b = to_double(right);
            return std::make_shared<DistributionConditional>(_real_distr, a, b);
        }
    private:
        virtual double to_double(T value) const = 0;
        virtual T from_double(double value) const = 0;
        
        std::shared_ptr<const Distribution> _real_distr;
    };

    template <class T> GenericDistributionFromReal<T>::GenericDistributionFromReal(std::shared_ptr<const Distribution> real_distr)
        : _real_distr(real_distr) {
        if (!_real_distr) {
            throw std::domain_error("GenericDistributionFromReal: null real distribution");
        }
    }    
}

#endif // __AVERISERA_GENERIC_DISTRIBUTION_FROM_REAL_H
