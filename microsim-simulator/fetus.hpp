#ifndef __AVERISERA_MICROSIM_FETUS_HPP
#define __AVERISERA_MICROSIM_FETUS_HPP

#include "microsim-core/person_attributes.hpp"
#include "core/dates.hpp"

namespace averisera {
    namespace microsim {
        /*! Fetus becomes a Person after being born. For now it is a POD class */
        class Fetus {
        public:
            /*!
              \param attributes Constant attributes such as sex, ethnicity.
              \param conception_date Conception date
            */
            Fetus(const PersonAttributes& attributes, Date conception_date)
                : _attributes(attributes), _conception_date(conception_date) {
            }

            const PersonAttributes& attributes() const {
                return _attributes;
            }

            Date conception_date() const {
                return _conception_date;
            }
        private:
            PersonAttributes _attributes;
            Date _conception_date;
        };

		std::ostream& operator<<(std::ostream& os, const Fetus& fetus);
    }
}

#endif // __AVERISERA_MICROSIM_FETUS_HPP
