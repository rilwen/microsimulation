#include "perturb_date_of_birth.hpp"
#include "../person_data.hpp"
#include "core/dates.hpp"
#include "core/math_utils.hpp"
#include "core/period.hpp"
#include <cassert>

namespace averisera {
	namespace microsim {
        static Date first_history_date(const PersonData& pd) {
            Date d = Date::MAX;
            for (const auto& kv : pd.histories) {
                if (!kv.second.dates().empty()) {
                    // assume that dates are in strictly increasing order
                    d = std::min(d, kv.second.dates().front());
                }
            }
            return d;
        }

        static void shift_all_dates(PersonData& pd, const Period& delta) {
            for (auto& kv: pd.histories) {
				kv.second.shift_dates(delta);                
            }
        }
        
        void PerturbDateOfBirth::apply(std::vector<PersonData>& datas, const Contexts& ctx) const {
            for (PersonData& data : datas) {
                if (_avoid_linked && data.mother_id != Actor::INVALID_ID) {
                    continue;
                }
                
                Date new_date_of_birth = perturb_date_of_birth(data.date_of_birth, ctx);
                const Period delta(PeriodType::DAYS, MathUtils::safe_cast<int>((new_date_of_birth - data.date_of_birth).days()));
                if (!_shift_history_dates) {
                    // try to fit in the new birth date
                    new_date_of_birth = std::min(new_date_of_birth, first_history_date(data));
                } else {
                    shift_all_dates(data, delta);
                }
                if (!data.conception_date.is_not_a_date()) {                    
                    Date new_conception_date;
                    try {
                        new_conception_date = data.conception_date + delta;
                    } catch (std::out_of_range&) {
                        // overflow detected... unlikely!
                        if (delta.size > 0) {
                            new_conception_date = Date::MAX;
                        } else {
                            new_conception_date = Date::MIN;
                        }
                    }
                    assert(!new_conception_date.is_not_a_date());
                    data.conception_date = new_conception_date;
                }
                if (data.mother_id != Actor::INVALID_ID) {
                    assert(!_avoid_linked);
                    const auto mother_it = ActorData::find_by_id(datas, data.mother_id);
                    if (mother_it != datas.end()) {
                        throw std::runtime_error("PerturbDateOfBirth: changing pregnancy history not implemented");
                    } else {
                        throw std::domain_error("PerturbDateOfBirth: no mother with this ID");
                    }
                }
                data.date_of_birth = new_date_of_birth;
            }
        }        
	}
}
