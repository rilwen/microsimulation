// (C) Averisera Ltd 2014-2020
#pragma once
#include "microsim-core/ethnicity.hpp"
#include "core/data_frame.hpp"
#include "core/numerical_range.hpp"
#include <cstdint>

namespace averisera {
	namespace microsim {
		namespace CalibrationTypes {
			typedef uint32_t age_type; /** Person's age */
			typedef NumericalRange<age_type> age_group_type; /**< [min, max) age at time of event (inclusive/exclusive). */
			typedef DataFrame<Ethnicity::index_set_type, age_group_type> pop_data_type; /** For census data */
		}
	}
}
