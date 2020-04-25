#ifndef __AVERISERA_MICROSIM_OPERATOR_PREGNANCY_HPP
#define __AVERISERA_MICROSIM_OPERATOR_PREGNANCY_HPP

#include "../history_generator_simple.hpp"
#include "../history_user_simple.hpp"
#include "../operator_individual.hpp"
#include "microsim-core/pregnancy.hpp"

namespace averisera {	
	template <class T> class Array2D;
    namespace microsim {
        template <class T> class OperatorMarkovModelActor;
        class Person;
        template <class T> class RelativeRisk;

		typedef uint8_t event_storage_type; /**< Value type of the History used to store pregnancy events */
        
        /** Operator for modelling the course of pregnancy. 
          
          If we switch operators during pregnancy to account e.g. for sudden onset of a disease, we should keep the same transition count structure.
         */
        class OperatorPregnancy: public OperatorIndividual<Person> {
        public:
            /**
              @param pregnancy Pregnancy model
              @param pred Predicate. It is multiplied internally with the predicate selecting only live females.
              @param relative_risks_transitions Vector of Array2D in which relative_risks_transitions[model_idx][from][to] is the relative risk to be applied to the transition for model_idx-th model. Each element is moved to OperatorPregnancy object.
			  @param min_childbearing_age Minimum childbearing age in years
			  @param max_childbearing_age Maximum childbearing age in years
              @throw std::domain_error If relative_risks_transitions.size() != pregnancy.nbr_stage_models(). If min_childbearing_age > max_childbearing_age. If max_childbearing_age == std::numeric_limits<unsigned int>::MAX().

            */
			OperatorPregnancy(Pregnancy&& pregnancy, std::shared_ptr<const Predicate<Person>> pred,
				std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>>&& relative_risks_transitions,
				unsigned int min_childbearing_age, unsigned int max_childbearing_age);

            /** Factory method constructing the operator on the heap with empty relative risks */
            static std::unique_ptr<OperatorPregnancy> build_with_empty_relative_risks(Pregnancy&& pregnancy, std::shared_ptr<const Predicate<Person>> pred, unsigned int min_childbearing_age, unsigned int max_childbearing_age);

            const Predicate<Person>& predicate() const override {
                return *_pred;
            }

            unsigned int nbr_stage_models() const {
                return _pregnancy.nbr_stage_models();
            }

            void apply(const std::shared_ptr<Person>& obj, const Contexts& contexts) const override;

			const HistoryGenerator<Person>::reqvec_t& requirements() const override {
				return hist_gen_.requirements();
			}

			const HistoryUser<Person>::use_reqvec_t& user_requirements() const override {
				return history_user_.user_requirements();
			}

			const std::string& name() const override {
				static const std::string str("Pregnancy");
				return str;
			}
        private:
			HistoryGeneratorSimple<Person> hist_gen_;
			HistoryUserSimple<Person> history_user_;
            Pregnancy _pregnancy;
            std::shared_ptr<const Predicate<Person>> _pred;
            std::vector<OperatorMarkovModelActor<Person>> _operators;
            std::vector<unsigned int> _cumulative_transition_counts;
			unsigned int min_childbearing_age_;
			unsigned int max_childbearing_age_;

			/** @param dummy Dummy argument to allow constructor overloading */
			OperatorPregnancy(Pregnancy&& pregnancy, std::shared_ptr<const Predicate<Person>> pred,
				std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>>&& relative_risks_transitions,
				unsigned int min_childbearing_age, unsigned int max_childbearing_age, bool dummy);

            std::pair<Date, Pregnancy::Event> get_last_date_and_event(const Person& obj, const Contexts& ctx) const;

			std::pair<Date, Pregnancy::State> get_last_date_and_state(const Person& obj, const Contexts& ctx) const;

            bool is_initialized(const Person& obj, const Contexts& contexts) const;

			/** Return date actually set */
            Date set_next_event(Person& obj, Date date, Pregnancy::Event evt, const Contexts& ctx) const;

            ImmutableHistory::index_t transitions_since_conception(const Person& obj, const Contexts& contexts, Date current_date) const;

			static std::shared_ptr<const Predicate<Person>> wrap(std::shared_ptr<const Predicate<Person>> external_predicate, unsigned int min_childbearing_age, unsigned int max_childbearing_age);

			static std::vector<Array2D<std::shared_ptr<const RelativeRisk<Person>>>> empty_relative_risks(const Pregnancy& pregnancy);

			static Pregnancy::State resulting_state(Pregnancy::Event evt) {
				return Pregnancy::resulting_state(evt, true);
			}
        };
    }
}

#endif // __AVERISERA_MICROSIM_OPERATOR_PREGNANCY_HPP
