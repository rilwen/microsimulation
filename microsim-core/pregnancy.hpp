#ifndef __AVERISERA_MS_PREGNANCY_H
#define __AVERISERA_MS_PREGNANCY_H

//#include "core/dates.hpp"
#include "markov_model.hpp"
#include <array>
#include <cassert>
#include <iosfwd>
#include <memory>
#include <vector>

namespace averisera {
    namespace microsim {
        /*! \brief Pregnancy model.
          
          We model pregnancy as beginning with conception (a jump process event) followed by repeated applications of one or more Markov models. Pregnancy class describes the course of the pregnancy after conception.

          Each Markov model describes transitions between predefined events as modelled by Pregnancy::Event enum class. All but the last Markov models
          are applied a fixed number of times, while the last one is applied indefinitely.

		  Events cause the pregnancy state to change between State::PREGNANT and State::NOT_PREGNANT.

          The model defines a list of "terminating states" which end the pregnancy.

          We assume that miscarriage ends pregnancy completely, regardless of its multiplicity.

		  \see Pregnancy::Event
		  \see Pregnancy::State
		  \see Conception
         */
        class Pregnancy {
        public:
            /*! Pregnancy events
             */
            enum class Event {
				CONCEPTION = 0, /*!< Event: conception */
                    MISCARRIAGE, /*!< Event: Pregnancy outcome: miscarriage (terminates pregnancy) */
                    BIRTH, /*!< Event: Pregnancy outcome: birth (terminates pregnancy) */
                    SIZE /*!< Use this constant to get the number of other constants */
                    };

			enum class State {
				NOT_PREGNANT = 0,
				PREGNANT,
				SIZE
			};

            /*! Array of terminating events: MISCARRIAGE and BIRTH */
            static const std::array<Event, 2> TERMINATING_EVENTS;

			typedef unsigned int size_type;
			typedef unsigned int transition_count_type;

            /*! Construct the pregnancy model.
              \param[in] markov_models Markov models for subsequent stages of pregnancy, generating transitions between Pregnancy::State values
              \param[in] transition counts How many transitions from each model; last model is applied indefinitely and thus needs no transition count.
              \throw If any of markov_models elements is null or has dim() != State::SIZE. If markov_models.size() > transition_counts.size() + 1. If no Markov models are provided.
             */
            Pregnancy(std::vector<std::unique_ptr<const MarkovModel>>&& markov_models,
                      const std::vector<transition_count_type> transition_counts                      
                );

			/*! For humans */
			static const unsigned int PREGNANCY_IN_MONTHS = 9;

			/*! Simplest model possible, in which every pregnancy lasts 9 months and ends with birth. */
			Pregnancy();

			/*! Is Event terminating? */
            static bool is_terminating(Event evt);

			/*! Return State resulting from Event happening 
			\param none_event_is_not_pregnant If true, Event::SIZE is translated to State::NOT_PREGNANT, else to Stat::SIZE  */
			static State resulting_state(Event evt, bool none_event_is_not_pregnant);

            /*! Move constructor */
            Pregnancy(Pregnancy&& other);
            
            Pregnancy(const Pregnancy&) = delete;
            Pregnancy& operator==(const Pregnancy&) = delete;

            /*! Number of models for transitions between pregnancy stages */
            size_type nbr_stage_models() const {
                return static_cast<size_type>(_markov_models.size());
            }

            /*! Return idx-th Markov model for pregnancy stages 
              \param idx Model index < nbr_stage_models()
             */
            const MarkovModel& stage_model(size_type idx) const {
                assert(idx < nbr_stage_models());
                assert(_markov_models[idx]);
                return *_markov_models[idx];                
            }

            /*! Number of transitions generated by the idx-th model 
              Undefined for the last model.
              \param idx Model index < nbr_stage_models() - 1
             */
            transition_count_type transition_count(size_type idx) const {
                assert(idx + 1 < nbr_stage_models());
                return _transition_counts[idx];
            }

            /*! Builder class */
            class Builder {
            public:
                Builder();
                Builder(const Builder& other) = delete;
                Builder& operator==(const Builder&) = delete;                

                void add_markov_model(std::unique_ptr<const MarkovModel>&& markov_model, transition_count_type transition_count);

                /*! Takes over ownership of markov_model */
                void add_markov_model(const MarkovModel* markov_model, transition_count_type transition_count) {
                    add_markov_model(std::unique_ptr<const MarkovModel>(markov_model), transition_count);
                }

                Pregnancy build();
            private:
                std::vector<std::unique_ptr<const MarkovModel>> _markov_models;
                std::vector<transition_count_type> _transition_counts;
            };
        private:
            std::vector<std::unique_ptr<const MarkovModel>> _markov_models;
            std::vector<transition_count_type> _transition_counts;
        };

		std::ostream& operator<<(std::ostream& os, Pregnancy::State state);
		std::ostream& operator<<(std::ostream& os, Pregnancy::Event state);
    }
}

#endif // __AVERISERA_MS_PREGNANCY_H