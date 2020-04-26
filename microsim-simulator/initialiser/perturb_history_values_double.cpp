// (C) Averisera Ltd 2014-2020
#include "../history_data.hpp"
#include "../person_data.hpp"
#include "perturb_history_values_double.hpp"
#include "core/stl_utils.hpp"
#include <cassert>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {
        template <class AD> PerturbHistoryValuesDouble<AD>::PerturbHistoryValuesDouble(const std::string& variable_name, double lower_bound, double upper_bound)
            : _variable_name(variable_name), _lower_bound(lower_bound), _upper_bound(upper_bound) {
            if (variable_name.empty()) {
                throw std::domain_error("PerturbHistoryValuesDouble: empty variable name");
            }
            if (!(lower_bound <= upper_bound)) {
                throw std::domain_error(boost::str(boost::format("PerturbHistoryValuesDouble: lower bound %g is not less than or equal to upper bound %g") % lower_bound % upper_bound));
            }
        }

        template <class AD> void PerturbHistoryValuesDouble<AD>::apply(AD& data, const Contexts& ctx) const {
            if (StlUtils::contains(data.histories, _variable_name)) {
                HistoryData& hd = data.histories[_variable_name];
                if (hd.values().type() == HistoryData::type_t::DOUBLE) {
                    assert(hd.values().size() == hd.dates().size());
                    size_t idx = 0;
                    for (const double& v : hd.values().as<double>()) {
                        double new_v = perturb(hd.dates()[idx], v, ctx);
                        new_v = std::min(new_v, _upper_bound);
                        new_v = std::max(new_v, _lower_bound);
						hd.set_value(idx, new_v);
                        ++idx;
                    }
                } else {
                    throw std::domain_error("PerturbHistoryValuesDouble: history type is not double");
                }
            }
        }

        template class PerturbHistoryValuesDouble<ActorData>;
        template class PerturbHistoryValuesDouble<PersonData>;
    }
}
