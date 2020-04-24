/*
(C) Averisera Ltd 2017
*/
#include "ethnicity_classifications_england_wales.hpp"
#include "ethnicity_ons_full.hpp"
#include "ethnicity_ons_major.hpp"
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		Ethnicity::IndexConversions EthnicityClassficationsEnglandWales::get_conversions(const std::string& name) {
			if (name == "") {
				return Ethnicity::IndexConversions();
			} else if (name == EthnicityONSFull::CLASSIFICATION_NAME) {
				return Ethnicity::IndexConversions::build<EthnicityONSFull>();
			} else if (name == EthnicityONSMajor::CLASSIFICATION_NAME) {
				return Ethnicity::IndexConversions::build<EthnicityONSMajor>();
			} else {
				throw DataException(boost::str(boost::format("EthnicityClassficationsEnglandWales: classification scheme %s uknown") % name));
			}
		}
	}
}
