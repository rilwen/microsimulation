// (C) Averisera Ltd 2014-2020
#include "operator_fetus_generator_simple.hpp"
#include "../contexts.hpp"
#include "../fetus.hpp"
#include "../mutable_context.hpp"
#include "../person.hpp"
#include "microsim-core/person_attributes.hpp"
#include "microsim-core/sex.hpp"
#include <stdexcept>
#include <vector>

namespace averisera {
    namespace microsim {
        OperatorFetusGeneratorSimple::OperatorFetusGeneratorSimple(MarkovModel&& ethnicity_model, probs_type&& probs_female, unsigned int min_childbearing_age, unsigned int max_childbearing_age)
            : OperatorFetusGenerator(nullptr, min_childbearing_age, max_childbearing_age), ethnicity_model_(std::move(ethnicity_model)) {
			if (probs_female.empty()) {
				throw std::domain_error("OperatorFetusGeneratorSimple: empty gender prob series");
			}
			for (const auto& tv : probs_female) {
				const double prob_female = tv.second;
				if (prob_female < 0 || prob_female > 1) {
					ethnicity_model = std::move(ethnicity_model_); // cleanup
					throw std::out_of_range("OperatorFetusGeneratorSimple: probability of female gender outside [0, 1]");
				}
			}
			probs_female_ = std::move(probs_female);
        }

        OperatorFetusGeneratorSimple::OperatorFetusGeneratorSimple(unsigned int min_childbearing_age, unsigned int max_childbearing_age, double prob_female)
            : OperatorFetusGeneratorSimple(MarkovModel(Eigen::MatrixXd::Identity(PersonAttributes::MAX_ETHNICITY, PersonAttributes::MAX_ETHNICITY)), probs_type(Date(2000, 1, 1), prob_female), min_childbearing_age, max_childbearing_age) {
        }

		OperatorFetusGeneratorSimple::OperatorFetusGeneratorSimple(probs_type&& probs_female, unsigned int min_childbearing_age, unsigned int max_childbearing_age)
			: OperatorFetusGeneratorSimple(MarkovModel(Eigen::MatrixXd::Identity(PersonAttributes::MAX_ETHNICITY, PersonAttributes::MAX_ETHNICITY)), std::move(probs_female), min_childbearing_age, max_childbearing_age) {
		}

        std::vector<Fetus> OperatorFetusGeneratorSimple::generate_fetuses(const std::shared_ptr<Person>& mother, const Date conception_date, Conception::multiplicity_type multiplicity, const Contexts& contexts) const {
            std::vector<Fetus> fetuses;
            fetuses.reserve(multiplicity);
            const unsigned int child_ethnicity = ethnicity_model_.select_next_state(mother->ethnicity(), contexts.mutable_ctx().rng().next_uniform());
            for (Conception::multiplicity_type i = 0; i < multiplicity; ++i) {
                const double u = contexts.mutable_ctx().rng().next_uniform();
                const Sex sex = (u <= probs_female_.padded_value(conception_date)) ? Sex::FEMALE : Sex::MALE;
                const PersonAttributes attributes(sex, MathUtils::safe_cast<PersonAttributes::ethnicity_t>(child_ethnicity));
				LOG_TRACE() << "OperatorFetusGeneratorSimple: generating fetus with attributes " << attributes << " conceived on " << conception_date;
                fetuses.push_back(Fetus(attributes, conception_date));
            }
            return fetuses;
        }
    }
}
