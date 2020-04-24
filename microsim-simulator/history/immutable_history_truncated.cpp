#include "immutable_history_truncated.hpp"
#include "core/dates.hpp"
#include <algorithm>
#include <limits>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {
    namespace microsim {
        ImmutableHistoryTruncated::ImmutableHistoryTruncated(const ImmutableHistory& original, Date end)
            : _original(original), _end(end) {
        }

        bool ImmutableHistoryTruncated::empty() const {
            if (_original.empty()) {
                return true;
            } else {
                const Date fd = _original.first_date();
                return fd > _end;	       		
            }
        }

        Date ImmutableHistoryTruncated::last_date() const {
            return last_date(_end);	
        }

        Date ImmutableHistoryTruncated::first_date() const {
            const Date fd = _original.first_date();
            if (fd > _end) {
                throw std::logic_error("ImmutableHistoryTruncated: history is empty");
            } else {
                return fd;
            }
        }

        Date ImmutableHistoryTruncated::last_date(Date asof) const {
            return _original.last_date(std::min(asof, _end));
        }
	
        double_t ImmutableHistoryTruncated::as_double(Date asof) const {
            if (asof <= _end) {
                return _original.as_double(asof);
            } else {
                return std::numeric_limits<double_t>::quiet_NaN();
            }
        }
	
        ImmutableHistoryTruncated::int_t ImmutableHistoryTruncated::as_int(Date asof) const {
            if (asof <= _end) {
                return _original.as_int(asof);
            } else {
                throw std::runtime_error("ImmutableHistoryTruncated: requesting int value after end");
            }
        }
	
        ImmutableHistoryTruncated::double_t ImmutableHistoryTruncated::last_as_double() const {
            return _original.last_as_double(_end);
        }
	
        ImmutableHistoryTruncated::int_t ImmutableHistoryTruncated::last_as_int() const {
            return _original.last_as_int(_end);
        }
	
        ImmutableHistoryTruncated::double_t ImmutableHistoryTruncated::last_as_double(Date asof) const {
            return _original.last_as_double(std::min(asof, _end));
        }
	
        ImmutableHistoryTruncated::int_t ImmutableHistoryTruncated::last_as_int(Date asof) const {
            return _original.last_as_int(std::min(asof, _end));
        }

        ImmutableHistoryTruncated::index_t ImmutableHistoryTruncated::size() const {
            try {
                const index_t li = last_index(_end);
                return li + 1;
            } catch (std::out_of_range&) {
                return 0;
            }
        }

        Date ImmutableHistoryTruncated::date(index_t idx) const {
            if (idx < size()) {
                return _original.date(idx);
            } else {
                throw std::out_of_range(boost::str(boost::format("ImmutableHistoryTruncated: index %d too large (size %d)") % idx % size()));
            }
        }

        ImmutableHistoryTruncated::double_t ImmutableHistoryTruncated::as_double(index_t idx) const {
            if (idx < size()) {
                return _original.as_double(idx);
            } else {
                throw std::out_of_range(boost::str(boost::format("ImmutableHistoryTruncated: index %d too large (size %d)") % idx % size()));
            }
        }

        ImmutableHistoryTruncated::int_t ImmutableHistoryTruncated::as_int(index_t idx) const {
            if (idx < size()) {
                return _original.as_int(idx);
            } else {
                throw std::out_of_range(boost::str(boost::format("ImmutableHistoryTruncated: index %d too large (size %d)") % idx % size()));
            }
        }

        ImmutableHistoryTruncated::index_t ImmutableHistoryTruncated::last_index(Date asof) const {
            return _original.last_index(asof >= _end ? _end : asof);
        }

        ImmutableHistoryTruncated::index_t ImmutableHistoryTruncated::first_index(Date asof) const {
            const index_t idx = _original.first_index(asof);
            if (idx < size()) {
                return idx;
            } else {
                throw std::out_of_range(boost::str(boost::format("ImmutableHistoryTruncated: no dates on or after %s") % boost::lexical_cast<std::string>(asof)));
            }
        }

        void ImmutableHistoryTruncated::print(std::ostream& os) const {
            os << "truncated " << _end << " " << _original;
        }

		HistoryData ImmutableHistoryTruncated::to_data() const {
			HistoryData data = _original.to_data();
			data.truncate_to(size());
			return data;
		}
    }
}
