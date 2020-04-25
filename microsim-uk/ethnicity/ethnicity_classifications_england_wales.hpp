#pragma once
/*
(C) Averisera Ltd 2017
*/
#include "microsim-core/ethnicity.hpp"
#include <string>

namespace averisera {
	namespace microsim {
		struct EthnicityClassficationsEnglandWales {
			/** Return Ethnicity::IndexConversions for given classification scheme, or throw DataException if not found 
			@param name Name of the classification scheme. If empty, return empty conversions object. 
			*/
			static Ethnicity::IndexConversions get_conversions(const std::string& name);
		};
	}
}
