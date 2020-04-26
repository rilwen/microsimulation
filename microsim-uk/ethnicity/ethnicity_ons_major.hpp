// (C) Averisera Ltd 2014-2020
#pragma once
#include "microsim-core/person_attributes.hpp"
#include <array>

namespace averisera {
	namespace microsim {
		/** Major ethnic group classification for ONS data */
		struct EthnicityONSMajor {
			enum class Group : PersonAttributes::ethnicity_t {
				WHITE = 0,
				MIXED,
				ASIAN,
				BLACK_AND_CARIBBEAN,
				OTHER,
				SIZE /**< Use it to get the number of other values */
			};

			/** Number of groups */
			static const size_t SIZE = static_cast<size_t>(Group::SIZE);

			/** Name of the classification scheme */
			static const char* const CLASSIFICATION_NAME;

			/** Names of groups */
			static const std::array<const char*, SIZE + 1> NAMES;
		};
	}
}
