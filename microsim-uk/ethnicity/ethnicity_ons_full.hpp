#pragma once
#include "microsim-core/ethnicity.hpp"
#include "microsim-core/person_attributes.hpp"
#include "ethnicity_ons_major.hpp"
#include <array>
#include <iosfwd>
#include <utility>

namespace averisera {
	namespace microsim {
		/*! Full ethnic group classification from ONS 2011 census */
		struct EthnicityONSFull {
			/*! Constants order first by major group, then by subgroup */
			enum class Group : PersonAttributes::ethnicity_t {
				WHITE_BRITISH = 0,
				IRISH,
				GYPSY_OR_TRAVELLER,
				OTHER_WHITE,
				WHITE_AND_BLACK_CARIBBEAN,
				WHITE_AND_ASIAN,
				WHITE_AND_BLACK_AFRICAN,
				OTHER_MIXED,
				INDIAN,
				PAKISTANI,
				BANGLADESHI,
				CHINESE,
				OTHER_ASIAN,
				AFRICAN,
				CARIBBEAN,
				OTHER_BLACK,
				ARAB,
				ANY_OTHER,
				SIZE /*!< Use it to get the number of other values */
			};

			/*! Number of groups */
			static const size_t SIZE = static_cast<size_t>(Group::SIZE);

			/*! Name of the classification scheme */
			static const char* const CLASSIFICATION_NAME;

			/*! Convert to "major" group
			If full == EthnicityONSFull::SIZE, return EthnicityONSMajor::SIZE
			\throw std::out_of_range If value unknown
			*/
			static EthnicityONSMajor::Group to_ons_major(Group full);

			typedef Ethnicity::range_type<EthnicityONSFull> range_type;

			/*! Return a range [min, max] */
			static range_type from_ons_major(EthnicityONSMajor::Group grp) {
				return MAJOR_RANGES[static_cast<size_t>(grp)];
			}

			/*! Names of groups */
			static const std::array<const char*, SIZE + 1> NAMES;

			/*! [min, max] ranges of detailed groups corresponding to major groups */
			static const std::array<range_type, EthnicityONSMajor::SIZE + 1> MAJOR_RANGES;
		};

		std::ostream& operator<<(std::ostream& os, EthnicityONSFull::Group group);
	}
}
