#include "../common_features.hpp"
#include "mortality.hpp"
#include "../contexts.hpp"
#include "../predicate_factory.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "core/utils.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {

		static void check_dim(const HazardModel& hazard_model) {
			if (hazard_model.dim() != 2) {
				throw std::domain_error("Mortality: bad hazard model dimension");
			}
		}

        Mortality::Mortality(const HazardModel& hazard_model, const std::vector<std::shared_ptr<const RelativeRisk<Person>>>& relative_risks, std::shared_ptr<const Predicate<Person>> predicate, std::unique_ptr<Schedule>&& schedule, bool move_to_birth_date)
			: OperatorHazardModel<Person>(CommonFeatures::MORTALITY(), Utils::pass_through(hazard_model, [&hazard_model]() { check_dim(hazard_model); }), relative_risks, predicate, std::move(schedule)),
			_move_to_birth_date(move_to_birth_date) {            			
        }

		Mortality::Mortality(HazardModel&& hazard_model, std::vector<std::shared_ptr<const RelativeRisk<Person>>>&& relative_risks, std::shared_ptr<const Predicate<Person>> predicate, std::unique_ptr<Schedule>&& schedule, bool move_to_birth_date)
			: OperatorHazardModel<Person>(CommonFeatures::MORTALITY(), std::move(Utils::pass_through(hazard_model, [&hazard_model]() { check_dim(hazard_model); })), std::move(relative_risks), predicate, std::move(schedule)), _move_to_birth_date(move_to_birth_date) {
		}

		Mortality::state_t Mortality::current_state(const Person& person, const Contexts& ctx) const {
            return person.is_alive(ctx.asof()) ? ALIVE : DEAD;
        }
        
        void Mortality::set_next_state(Person& person, Date date, state_t state, const Contexts&) const {
            if (state == DEAD) {
                person.die(date);
            } // else do nothing
        }

		std::unique_ptr<HazardModel> Mortality::adapt_hazard_model(const Person& obj) const {
			if (_move_to_birth_date) {
				return std::unique_ptr<HazardModel>(new HazardModel(hazard_model().move(obj.date_of_birth())));
			} else {
				return nullptr;
			}
		}

		std::vector<std::unique_ptr<Mortality>> Mortality::build_operators(std::vector<std::unique_ptr<AnchoredHazardCurve>>&& mortality_curves, const Schedule* schedule, const std::shared_ptr<const Predicate<Person>>& predicate) {
			std::vector<std::unique_ptr<Mortality>> operators(mortality_curves.size());
			std::transform(mortality_curves.begin(), mortality_curves.end(), operators.begin(), [schedule, &predicate](std::unique_ptr<AnchoredHazardCurve>& curve) {
				if (curve != nullptr) {
					const int year = curve->start().year(); 
					std::vector<std::shared_ptr<const AnchoredHazardCurve>> shared_curves({ std::move(curve), nullptr });
					std::vector<unsigned int> next_states({ 1, 1 });
					HazardModel hazard_model(std::move(shared_curves), std::move(next_states));					
					std::unique_ptr<Schedule> opsched;
					if (schedule) {
						opsched = std::make_unique<Schedule>(*schedule);
					}
					std::shared_ptr<const Predicate<Person>> predicate_for_operator = PredicateFactory::make_year_of_birth(year, year, true);
					if (predicate) {
						predicate_for_operator = PredicateFactory::make_and({ predicate, predicate_for_operator });
					}
					return std::make_unique<Mortality>(std::move(hazard_model), std::vector<std::shared_ptr<const RelativeRisk<Person>>>(), predicate_for_operator, std::move(opsched), true);
				} else {
					throw std::domain_error("MortalityCalibrator: null mortality curve");
				}
			});
			return operators;
		}
    }
}
