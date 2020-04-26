// (C) Averisera Ltd 2014-2020
#include "anchored_hazard_curve.hpp"
#include "conception.hpp"
#include "hazard_model.hpp"
#include "core/generic_distribution.hpp"
#include "core/generic_distribution_integral.hpp"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
    namespace microsim {
        Conception::Conception(std::shared_ptr<const AnchoredHazardCurve> conception_curve, mdistr_multi_series_type&& multiplicity_distros)
            : _conception_curve(conception_curve), _move_to_dob(true) {
            if (!conception_curve) {
                throw std::domain_error("Conception: null conception risk curve");
            }
            if (multiplicity_distros.empty()) {
                throw std::domain_error("Conception: empty multiplicity distributions time series series");
            }
			for (const auto& tts : multiplicity_distros) {
				const auto& ts = tts.second;
				if (ts.first_time() != 0) {
					throw std::domain_error(boost::str(boost::format("Conception: first time in time series should be zero, is: %g") % ts.first_time()));
				}
				for (const auto& tv : ts) {
					const auto& p = tv.second;
					if (!p || p->lower_bound() < 1) {
						throw std::domain_error(boost::str(boost::format("Conception: distribution at %s is either null or has lower bound below 1") % boost::lexical_cast<std::string>(tv.first)));
					}
				}
			}
            
            _multiplicity_distros = std::move(multiplicity_distros);
        }

		Conception::Conception(std::shared_ptr<const AnchoredHazardCurve> conception_curve)
			: Conception(conception_curve, std::make_shared < GenericDistributionIntegral<uint8_t>>(1, 1, 1)) {}
        
        Conception::Conception(Conception&& other) noexcept
            : _conception_curve(other._conception_curve), _multiplicity_distros(std::move(other._multiplicity_distros)), _move_to_dob(other._move_to_dob) {
        }

        HazardModel Conception::hazard_model(Date date_of_birth) const {
            std::shared_ptr<const AnchoredHazardCurve> curve_to_use;
            if (_move_to_dob) {
                curve_to_use = std::shared_ptr<const AnchoredHazardCurve>(_conception_curve->move(date_of_birth));
            } else {
                curve_to_use = _conception_curve;
            }
            assert(curve_to_use);
            return HazardModel({curve_to_use, nullptr}, {1, 1});
        }
    }
}
