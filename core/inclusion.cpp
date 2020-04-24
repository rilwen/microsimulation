#include "inclusion.hpp"
#include <ostream>
#include <boost/format.hpp>

namespace averisera {
	std::ostream& operator<<(std::ostream& os, InclusionRelation rel) {
		switch (rel) {
		case InclusionRelation::CONTAINS:
			os << "CONTAINS";
			break;
		case InclusionRelation::IS_CONTAINED_BY:
			os << "IS_CONTAINED_BY";
			break;
		case InclusionRelation::EQUALS:
			os << "EQUALS";
			break;
		case InclusionRelation::DISJOINT:
			os << "DISJOINT";
			break;
		case InclusionRelation::UNDEFINED:
			os << "UNDEFINED";
			break;
		default:
			throw std::logic_error(boost::str(boost::format("InclusionRelation: unknown enum: %d") % static_cast<int>(rel)));
		}
		return os;
	}
}
