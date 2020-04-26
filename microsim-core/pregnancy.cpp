// (C) Averisera Ltd 2014-2020
#include "pregnancy.hpp"
#include <algorithm>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        Pregnancy::Pregnancy(std::vector<std::unique_ptr<const MarkovModel>>&& markov_models,
                             const std::vector<transition_count_type> transition_counts
            )
            : _markov_models(std::move(markov_models)),
              _transition_counts(transition_counts) {
            if (_markov_models.empty()) {
                throw std::domain_error("Pregnancy: no Markov models");
            }
            if (std::any_of(_markov_models.begin(), _markov_models.end(), [](const std::unique_ptr<const MarkovModel>& m) {
                        return (!m) || (m->dim() != static_cast<unsigned int>(State::SIZE));
                    })) {
                throw std::domain_error("Pregnancy: null Markov model or bad model dimension");
            }
            if (_transition_counts.size() + 1 < _markov_models.size()) {
                throw std::domain_error("Pregnancy: bad vector sizes");
            }
            _transition_counts.resize(_markov_models.size() - 1);
        }

		Pregnancy::Pregnancy()
			: _markov_models(1)
		{
			static const size_t dim = static_cast<size_t>(Pregnancy::Event::SIZE);
			Eigen::MatrixXd transition_matrix(dim, dim);
			transition_matrix.setZero();
			static const size_t c = static_cast<size_t>(Pregnancy::Event::CONCEPTION);
			static const size_t m = static_cast<size_t>(Pregnancy::Event::MISCARRIAGE);
			static const size_t b = static_cast<size_t>(Pregnancy::Event::BIRTH);
			transition_matrix(b, c) = 1.0; // conception always results in birth
			transition_matrix(b, b) = 1.0; // birth is a terminating state
			transition_matrix(m, m) = 1.0; // miscarriage is a terminating state
			Eigen::VectorXd initial_state_probs(dim);
			initial_state_probs.setZero();
			initial_state_probs[c] = 1.0; // pro forma
			std::vector<Period> transition_periods(dim);
			transition_periods[c] = Period::months(9);
			transition_periods[b] = Period();
			transition_periods[m] = Period();
			_markov_models[0] = std::make_unique<MarkovModel>(std::move(transition_matrix), std::move(transition_periods), std::move(initial_state_probs));
		}
        
        Pregnancy::Pregnancy(Pregnancy&& other)
            : _markov_models(std::move(other._markov_models)),
            _transition_counts(std::move(other._transition_counts)) {
            other._markov_models.resize(0);
            other._transition_counts.resize(0);
        }

        Pregnancy::Builder::Builder()
        {}

        void Pregnancy::Builder::add_markov_model(std::unique_ptr<const MarkovModel>&& markov_model, unsigned int transition_count) {
            _markov_models.push_back(std::move(markov_model));
            _transition_counts.push_back(transition_count);
        }
        
        Pregnancy Pregnancy::Builder::build() {
            return Pregnancy(std::move(_markov_models), _transition_counts);
        }

        const std::array<Pregnancy::Event,2> Pregnancy::TERMINATING_EVENTS = {Event::BIRTH, Event::MISCARRIAGE};

        bool Pregnancy::is_terminating(Pregnancy::Event evt) {
            return std::find(TERMINATING_EVENTS.begin(), TERMINATING_EVENTS.end(), evt) != TERMINATING_EVENTS.end();
        }

		Pregnancy::State Pregnancy::resulting_state(Pregnancy::Event evt, const bool none_event_is_not_pregnant) {
			switch (evt) {
			case Pregnancy::Event::CONCEPTION:
				return Pregnancy::State::PREGNANT;
			case Pregnancy::Event::MISCARRIAGE:
			case Pregnancy::Event::BIRTH:
				return Pregnancy::State::NOT_PREGNANT;
			case Pregnancy::Event::SIZE:
				return none_event_is_not_pregnant ? Pregnancy::State::NOT_PREGNANT : Pregnancy::State::SIZE;
			default:
				throw std::out_of_range("Pregnancy::resulting_state: unknown event value");
			}
		}

		std::ostream& operator<<(std::ostream& os, Pregnancy::State state) {
			switch (state) {
			case Pregnancy::State::NOT_PREGNANT:
				os << "NOT_PREGNANT";
				break;
			case Pregnancy::State::PREGNANT:
				os << "PREGNANT";
				break;
			case Pregnancy::State::SIZE:
				os << "NONE";
				break;
			default:
				throw std::out_of_range("Pregnancy::State: unknown value");
			}
			return os;
		}

		std::ostream& operator<<(std::ostream& os, Pregnancy::Event evt) {
			switch (evt) {
			case Pregnancy::Event::CONCEPTION:
				os << "CONCEPTION";
				break;
			case Pregnancy::Event::MISCARRIAGE:
				os << "MISCARRIAGE";
				break;
			case Pregnancy::Event::BIRTH:
				os << "BIRTH";
				break;
			case Pregnancy::Event::SIZE:
				os << "NONE";
				break;
			default:
				throw std::out_of_range("Pregnancy::Event: unknown value");
			}
			return os;
		}
    }
}
