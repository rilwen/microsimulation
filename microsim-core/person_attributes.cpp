/*
 (C) Averisera Ltd 2015
 */
#include "person_attributes.hpp"
#include "sex.hpp"
#include <cassert>
#include <functional>

namespace averisera {
    namespace microsim {
        PersonAttributes::PersonAttributes()
            : PersonAttributes(Sex::FEMALE, 0) {
        }

        /*PersonAttributes::PersonAttributes(Sex sex, ethnicity_t ethnicity)
            : _sex(static_cast<storage_t>(sex) & 1) {
            assert(ethnicity <= MAX_ETHNICITY);
            _ethnicity = static_cast<storage_t>(ethnicity) & MAX_ETHNICITY;
        }*/

		PersonAttributes::PersonAttributes(Sex sex, ethnicity_t ethnicity)
			: _sex(sex), _ethnicity(ethnicity) {
			assert(ethnicity <= MAX_ETHNICITY);			
		}
        
        bool PersonAttributes::operator==(const PersonAttributes& other) const {
            return _sex == other._sex && _ethnicity == other._ethnicity;
        }

		bool PersonAttributes::operator<(const PersonAttributes& other) const {
			if (_sex != other._sex) {
				return _sex < other._sex;
			} else {
				return _ethnicity < other._ethnicity;
			}
		}

		std::ostream& operator<<(std::ostream& os, const PersonAttributes& data) {
			os << "(" << data.sex() << ", " << static_cast<int>(data.ethnicity()) << ")";
			return os;
		}

        // put the values in library file
        const PersonAttributes::ethnicity_t PersonAttributes::MAX_ETHNICITY;
        const unsigned int PersonAttributes::DIM;
    }
}

namespace std {
	hash<averisera::microsim::PersonAttributes>::result_type hash<averisera::microsim::PersonAttributes>::operator()(const argument_type& arg) const {
		//return std::hash<averisera::microsim::PersonAttributes::storage_t>()(reinterpret_cast<const averisera::microsim::PersonAttributes::storage_t&>(arg));
		return std::hash<uint8_t>()(static_cast<uint8_t>(arg.sex())) + 37 * std::hash<averisera::microsim::PersonAttributes::ethnicity_t>()(arg.ethnicity());
	}
}
