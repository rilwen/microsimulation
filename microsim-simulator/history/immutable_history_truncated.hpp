// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_MS_IMMUTABLE_HISTORY_TRUNCATED_H
#define __AVERISERA_MS_IMMUTABLE_HISTORY_TRUNCATED_H

#include "../immutable_history.hpp"
#include "core/dates.hpp"

namespace averisera {
    namespace microsim {
	/** @brief ImmutableHistory containing all events up to and including a certain date from another history.
      For temporary use (stores a reference to original history).
	 */
	class ImmutableHistoryTruncated: public ImmutableHistory {
	public:
	    /** @param[in] original Original history before truncation. Only reference is stored.
	      @param[in] end Truncation date
	    */
	    ImmutableHistoryTruncated(const ImmutableHistory& original, Date end);

        ImmutableHistoryTruncated& operator=(const ImmutableHistoryTruncated&) = delete;

        ImmutableHistoryTruncated(const ImmutableHistoryTruncated&) = delete;

	    bool empty() const override;

	    Date last_date() const override;

	    Date first_date() const override;

	    Date last_date(Date asof) const override;

	    double_t as_double(Date asof) const override;

	    int_t as_int(Date asof) const override;

	    double_t last_as_double() const override;

	    int_t last_as_int() const override;

	    double_t last_as_double(Date asof) const override;

	    int_t last_as_int(Date asof) const override;

        index_t size() const override;

        Date date(index_t idx) const override;

        double_t as_double(index_t idx) const override;

        int_t as_int(index_t idx) const override;

        index_t last_index(Date asof) const override;

        index_t first_index(Date asof) const override;

        void print(std::ostream& os) const override;

		HistoryData to_data() const override;

		const std::string& name() const override {
			return _original.name();
		}
	private:
	    const ImmutableHistory& _original;
	    Date _end;
	};
    }
}

#endif // __AVERISERA_MS_IMMUTABLE_HISTORY_TRUNCATED_H
