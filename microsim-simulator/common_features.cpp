// (C) Averisera Ltd 2014-2020
#include "common_features.hpp"
#include "feature.hpp"

namespace averisera {
	namespace microsim {
		namespace CommonFeatures {
			const Feature& MORTALITY() {
				static const Feature feat("MORTALITY");
				return feat;
			}
		}
	}
}
