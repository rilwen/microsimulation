#include "ethnicity_ons_major.hpp"

namespace averisera {
	namespace microsim {
		const char* const EthnicityONSMajor::CLASSIFICATION_NAME = "ONS_MAJOR";

		const std::array<const char*, EthnicityONSMajor::SIZE + 1> EthnicityONSMajor::NAMES = {
			"WHITE",
			"MIXED",
			"ASIAN",
			"BLACK_AND_CARIBBEAN",
			"OTHER",
			"" /*!< SIZE */
		};
	}
}
