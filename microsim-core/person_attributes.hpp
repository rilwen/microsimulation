/*
 (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_PERSON_ATTRIBUTES_H
#define __AVERISERA_MS_PERSON_ATTRIBUTES_H

#include "ethnicity.hpp"
#include "sex.hpp"
#include <cstdint>
#include <iosfwd>

namespace averisera {
    namespace microsim {
        /** \class PersonAttributes 
         * @brief Containts constant attributes of a person. Designed to use little memory.
         * 
         * Attributes:
         *  - sex (enum)
         *  - ethnicity (unsigned int in 0..127 range)
         */
        class PersonAttributes {
        public:
			//typedef uint8_t storage_t;
			typedef Ethnicity::group_index_type ethnicity_t;
            static const ethnicity_t MAX_ETHNICITY = static_cast<size_t>(Ethnicity::MAX_SIZE - 1); /**< Maximum value of ethnicity attribute. */
            static const unsigned int DIM = 2; /**< Number of values in PersonAttributes */

            PersonAttributes();
            
            /**
             * @param[in] sex Sex of the person
             * @param[in] ethnicity Index of the person's ethnic group
             */
            PersonAttributes(Sex sex, ethnicity_t ethnicity);
            
            /** @return Sex of the person (female or male)
             */
            Sex sex() const {
                return _sex;
            }
            
            /** @return Index of ethnic group.
             */
			ethnicity_t ethnicity() const {
                return _ethnicity;
            }
            
            /** Equality operator */
            bool operator==(const PersonAttributes& other) const;            

			bool operator<(const PersonAttributes& other) const;

			bool operator<=(const PersonAttributes& other) const {
				return !(other < *this);
			}

			bool operator>(const PersonAttributes& other) const {
				return other < *this;
			}

			friend std::ostream& operator<<(std::ostream& os, const PersonAttributes& data);
        private:
			/*
			// Use bitfields to squash values together.
            storage_t _sex : 1; 
            storage_t _ethnicity : 7;
			*/
			Sex _sex;
			ethnicity_t _ethnicity;
        };
    }
}

namespace std {
	template <> struct hash<averisera::microsim::PersonAttributes> {
		typedef averisera::microsim::PersonAttributes argument_type;
		typedef size_t result_type;
		result_type operator()(const argument_type& arg) const;
	};
}

#endif // __AVERISERA_MS_PERSON_ATTRIBUTES_H
