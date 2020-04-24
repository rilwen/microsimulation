#pragma once
#include "data_perturbation_individual.hpp"
#include "core/dates.hpp"
#include <string>

namespace averisera {
    namespace microsim {
        /*!
          Applies a perturbation to each value in a history (type = double).
          \tparam ActorData or derived class
        */
        template <class AD> class PerturbHistoryValuesDouble: public DataPerturbationIndividual<AD>  {
        public:
            /*! \param variable_name Variable to be perturbed
              \param lower_bound Lower bound for the perturbed value
              \param upper_bound Upper bound for the perturbed value
              \throw std::domain_error If lower_bound is not less than or equal the upper_bound. If variable_name is empty
            */
            PerturbHistoryValuesDouble(const std::string& variable_name, double lower_bound, double upper_bound);
        private:
            void apply(AD& data, const Contexts& ctx) const override;
            // Perturb a value 
            virtual double perturb(Date date, double value, const Contexts& ctx) const = 0;

            std::string _variable_name;
            double _lower_bound;
            double _upper_bound;
        };
    }
}
