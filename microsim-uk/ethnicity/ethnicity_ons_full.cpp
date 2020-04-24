#include "ethnicity_ons_full.hpp"
#include "ethnicity_ons_major.hpp"
#include <stdexcept>

namespace averisera {
	namespace microsim {
		const char* const EthnicityONSFull::CLASSIFICATION_NAME = "ONS_FULL";

		EthnicityONSMajor::Group EthnicityONSFull::to_ons_major(const EthnicityONSFull::Group full) {
			switch (full) {
			case EthnicityONSFull::Group::WHITE_BRITISH:
			case EthnicityONSFull::Group::IRISH:
			case EthnicityONSFull::Group::GYPSY_OR_TRAVELLER:
			case EthnicityONSFull::Group::OTHER_WHITE:
				return EthnicityONSMajor::Group::WHITE;
			case EthnicityONSFull::Group::WHITE_AND_BLACK_CARIBBEAN:
			case EthnicityONSFull::Group::WHITE_AND_ASIAN:
			case EthnicityONSFull::Group::WHITE_AND_BLACK_AFRICAN:
			case EthnicityONSFull::Group::OTHER_MIXED:
				return EthnicityONSMajor::Group::MIXED;
			case EthnicityONSFull::Group::INDIAN:
			case EthnicityONSFull::Group::PAKISTANI:
			case EthnicityONSFull::Group::BANGLADESHI:
			case EthnicityONSFull::Group::CHINESE:
			case EthnicityONSFull::Group::OTHER_ASIAN:
				return EthnicityONSMajor::Group::ASIAN;
			case EthnicityONSFull::Group::AFRICAN:
			case EthnicityONSFull::Group::CARIBBEAN:
			case EthnicityONSFull::Group::OTHER_BLACK:
				return EthnicityONSMajor::Group::BLACK_AND_CARIBBEAN;
			case EthnicityONSFull::Group::ARAB:
			case EthnicityONSFull::Group::ANY_OTHER:
				return EthnicityONSMajor::Group::OTHER;
			case EthnicityONSFull::Group::SIZE:
				return EthnicityONSMajor::Group::SIZE;
			default:
				throw std::out_of_range("EthnicityONSFull: unknown value");
			}
		}

		const std::array<const char*, EthnicityONSFull::SIZE + 1> EthnicityONSFull::NAMES = {
			"WHITE_BRITISH", // 0
			"IRISH", // 1
			"GYPSY_OR_TRAVELLER",
			"OTHER_WHITE", // 3
			"WHITE_AND_BLACK_CARIBBEAN",
			"WHITE_AND_ASIAN", // 5
			"WHITE_AND_BLACK_AFRICAN",
			"OTHER_MIXED",
			"INDIAN",
			"PAKISTANI",
			"BANGLADESHI", // 10
			"CHINESE",
			"OTHER_ASIAN",
			"AFRICAN",
			"CARIBBEAN",
			"OTHER_BLACK", // 15
			"ARAB",
			"ANY_OTHER",
			"" /*!< SIZE */
		};

		const std::array<EthnicityONSFull::range_type, EthnicityONSMajor::SIZE + 1> EthnicityONSFull::MAJOR_RANGES = {
			EthnicityONSFull::range_type(EthnicityONSFull::Group::WHITE_BRITISH, EthnicityONSFull::Group::OTHER_WHITE),
			EthnicityONSFull::range_type(EthnicityONSFull::Group::WHITE_AND_BLACK_CARIBBEAN, EthnicityONSFull::Group::OTHER_BLACK),
			EthnicityONSFull::range_type(EthnicityONSFull::Group::INDIAN, EthnicityONSFull::Group::OTHER_ASIAN),
			EthnicityONSFull::range_type(EthnicityONSFull::Group::AFRICAN, EthnicityONSFull::Group::OTHER_BLACK),
			EthnicityONSFull::range_type(EthnicityONSFull::Group::ARAB, EthnicityONSFull::Group::ANY_OTHER),
			EthnicityONSFull::range_type(EthnicityONSFull::Group::SIZE, EthnicityONSFull::Group::SIZE)
		};

		std::ostream& operator<<(std::ostream& os, EthnicityONSFull::Group group) {
			os << Ethnicity::get_name<EthnicityONSFull>(group);
			return os;
		}
	}
}
