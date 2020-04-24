#include "pred_pregnancy.hpp"
#include "../contexts.hpp"
//#include "core/log.hpp"

namespace averisera {
    namespace microsim {
        PredPregnancy::PredPregnancy(const Pregnancy::State state, bool alive, bool at_start)
            : _state(state), _alive(alive), at_start_(at_start) {
            if (state == Pregnancy::State::SIZE) {
                throw std::domain_error("PredPregnancy: invalid state selected");
            }
        }

        bool PredPregnancy::select_out_of_context(const Person& obj) const {
            return obj.sex() == Sex::FEMALE;
        }

        bool PredPregnancy::select(const Person& obj, const Contexts& contexts) const {
            if (select_out_of_context(obj)) {
				//LOG_TRACE() << "PredPregnancy::select: super_select=" << super_select;
                if (_alive) {
					return obj.is_alive(contexts.asof()) && is_pregnant(obj, contexts);
                } else {
                    return is_pregnant(obj, contexts);
                }
            } else {
                return false;
            }
        }

		bool PredPregnancy::select_alive(const Person& obj, const Contexts& contexts) const {
			return select_out_of_context(obj) && is_pregnant(obj, contexts);
		}

		bool PredPregnancy::is_pregnant(const Person& obj, const Contexts& contexts) const {
			const auto hist_idx = obj.get_variable_index(contexts, Procreation::PREGNANCY_EVENT());
			const ImmutableHistory& history = obj.history(hist_idx);
			const Date asof = at_start_ ? contexts.current_period().begin : contexts.current_period().end;
			if (history.empty() || history.first_date() > asof) {
				return _state == Pregnancy::State::NOT_PREGNANT;
			} else {
				const auto evt = static_cast<Pregnancy::Event>(history.last_as_int(asof));
				return _state == Pregnancy::resulting_state(evt, true);
			}
		}

		void PredPregnancy::print(std::ostream& os) const {
			os << "Pregnancy(" << static_cast<int>(_state) << ", " << _alive << ", " << at_start_ << ")";
		}
    }
}
