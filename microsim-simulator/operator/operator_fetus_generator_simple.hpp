// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MICROSIM_OPERATOR_FETUS_GENERATOR_SIMPLE_HPP
#define __AVERISERA_MICROSIM_OPERATOR_FETUS_GENERATOR_SIMPLE_HPP

#include "operator_fetus_generator.hpp"
#include "microsim-core/markov_model.hpp"
#include "core/dates.hpp"
#include "core/time_series.hpp"

namespace averisera {
    namespace microsim {
        /** Simple generation of fetuses: assigns sex with fixed probability, draws the ethnicity of the children based on the ethnicity of the mother using a Markov model (dodgy! no correlation between multiple pregnancies.
          For Predicate requirements and defaults, @see Operatorchildgenerator
        */
        class OperatorFetusGeneratorSimple: public OperatorFetusGenerator {
        public:
			typedef TimeSeries<Date, double> probs_type;

            /** For every mother age and multiplicity, draw boys and girls with fixed probability. Ethnicity is drawn from Markov model conditioned on mother's ethnicity (to account for mixed marriages). 
              @param probs_female Time-dependent female gender probability (as function of conception date)              
			  @throw std::out_of_range If any p in probs_female not in [0, 1] range.
			  @throw std::domain_error If probs_female is empty
              If exception is thrown, ethnicity_model is unchanged.
             */
            OperatorFetusGeneratorSimple(MarkovModel&& ethnicity_model, probs_type&& probs_female, unsigned int min_childbearing_age, unsigned int max_childbearing_age);

            /** Always inherit mother's ethnicity.
			@throw std::out_of_range If prob_female not in [0, 1] range.
			*/
            OperatorFetusGeneratorSimple(unsigned int min_childbearing_age, unsigned int max_childbearing_age, double prob_female = 0.5);

			/** Always inherit mother's ethnicity.  
			@param probs_female Time-dependent female gender probability (as function of conception date)
			@throw std::out_of_range If any p in probs_female not in [0, 1] range.
			*/
			OperatorFetusGeneratorSimple(probs_type&& probs_female, unsigned int min_childbearing_age, unsigned int max_childbearing_age);

			const std::string& name() const override {
				static const std::string str("FetusGeneratorSimple");
				return str;
			}
        private:
            std::vector<Fetus> generate_fetuses(const std::shared_ptr<Person>& mother, Date conception_date, Conception::multiplicity_type multiplicity, const Contexts& contexts) const override;

            MarkovModel ethnicity_model_;
            probs_type probs_female_;
        };
    }
}

#endif // __AVERISERA_MICROSIM_OPERATOR_FETUS_GENERATOR_SIMPLE_HPP
