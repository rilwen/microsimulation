/*
 * (C) Averisera Ltd 2015
 */
#ifndef __AVERISERA_MS_SCHEDULE_H
#define __AVERISERA_MS_SCHEDULE_H

#include "schedule_period.hpp"
#include <algorithm>
#include <memory>
#include <numeric>
#include <vector>
#include "core/dates_fwd.hpp"
#include "core/preconditions.hpp"

namespace averisera {
    class Daycount;
    namespace microsim {
        struct ScheduleDefinition;
        
        
        /*! \brief Describes the schedule of a simulation */
        class Schedule {
        public:
            typedef SchedulePeriod period_t; /*!< Period type */
            typedef std::vector<period_t>::const_iterator const_iterator;
			typedef size_t index_t;
            
            /*! Empty schedule */
            Schedule();

            Schedule(const Schedule&) = default;

            /*! Move constructor */
            Schedule(Schedule&& other) noexcept;
            
            /*! Construct a schedule from a definition.
             * \throw std::domain_error If schedule definition is invalid.
             */
            explicit Schedule(const ScheduleDefinition& definition);

            /*! Quick and dirty schedule from a series of dates. If passed vector of size 1, 
              constructs a 1-period Schedule with start = end.
              \throw std::domain_error If dates is empty or dates are not strictly increasing
             */
            explicit Schedule(const std::vector<Date>& dates);
            
            /*! Verify that the ScheduleDefinition is valid. */
            static bool valid(const ScheduleDefinition& definition);
            
            /*! Const iterator to first period */
            const_iterator begin() const {
                return _periods.begin();
            }
            
            /*! Const iterator to one past last period */
            const_iterator end() const {
                return _periods.end();
            }
            
            /*! Number of periods */
            index_t size() const {
                return _size;
            }

            /*! Number of dates */
            index_t nbr_dates() const {
                return _nbr_dates;
            }
            
            /*! Start date of the schedule */
            Date start_date() const {
                return _periods.front().begin;
            }
            
            /*! End date of the schedule */
            Date end_date() const {
                return _periods.back().end;
            }
            
            /*! Return a period
             * \param[in] idx Period index
             */
            const period_t& operator[](index_t idx) const {				
				assert(idx < _periods.size());
                return _periods[idx];
            }
            
            /*! Return i-th date, where 0 <= i <= size() 
             * Does not check the index value for speed.
             */
            Date date(index_t idx) const;

            /*! Is the schedule empty? */
            bool empty() const {
                return _periods.empty();
            }

            /*! Return the index of this date in the schedule.              
              \throw std::out_of_range If the date is not in the schedule */
            index_t index(Date date) const;

			/*! Return such i that date(i) <= d < date(i + 1) or throw std::out_of_range if not found */
			index_t find_containing_period(Date d) const;

            /*! Is the date in the schedule? */
            bool contains(Date date) const;

            /*! Is every date in other schedule contained in this? */
            bool contains(const Schedule& other) const;

			bool operator==(const Schedule& other) const;

            /*! Return a vector of years in which the schedule has simulation dates. */
            template <class YearType> std::vector<YearType> get_years() const {
				const size_t max_nbr_years = static_cast<size_t>(end_date().year() - start_date().year());
                std::vector<YearType> years;
				years.reserve(max_nbr_years);
				get_years(years);
				years.shrink_to_fit();
				return years;
            }

			/*! Return a vector of years in which the schedule has simulation dates. */
			template <class YearType> void get_years(std::vector<YearType>& years) const {
				for (index_t i = 0; i < nbr_dates(); ++i) {
					const YearType y = static_cast<YearType>(date(i).year());
					if (years.empty() || y > years.back()) {
						years.push_back(y);
					}
				}				
			}

			/*! Given a vector of years Y0, Y1, ... extend it back in time by adding Y0-how_much, Y0-how_much+1, ... in front */
			template <class YearType, class I> static std::vector<YearType> extend_back(const std::vector<YearType>& years, const I how_much) {
				check_that(!years.empty(), "Years vector is empty");
				std::vector<YearType> new_years(years.size() + how_much);
				std::iota(new_years.begin(), new_years.begin() + how_much, years.front() - how_much);
				std::copy(years.begin(), years.end(), new_years.begin() + how_much);
				return new_years;
			}
        private:
            std::vector<period_t> _periods;
            index_t _size;
			index_t _nbr_dates;
        };
    }
}

#endif // __AVERISERA_MS_SCHEDULE_H
