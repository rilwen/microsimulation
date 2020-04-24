#pragma once
#include "perturb_history_values_double.hpp"
#include <memory>

namespace averisera {
    class Distribution;
    namespace microsim {
        /*! Perturbs X returning X' = X + P, where P has given distribution. */
        template <class AD> class PerturbHistoryValuesDoubleLinear: public PerturbHistoryValuesDouble<AD> {
        public:
            /*! \param distr Perturbation distribution
              \throw std::domain_error If distr is null.
              \see PerturbHistoryValuesDouble
            */
            PerturbHistoryValuesDoubleLinear(const std::string& variable_name, double lower_bound, double upper_bound, std::unique_ptr<const Distribution>&& distr);
        private:
            double perturb(Date date, double value, const Contexts& ctx) const override;
            
            std::unique_ptr<const Distribution> _distr;
        };
    }
}
