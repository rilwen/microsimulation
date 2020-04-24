/*
(C) Averisera Ltd 2015
*/
#include "contexts.hpp"
#include "immutable_context.hpp"
#include "mutable_context.hpp"
#include "core/daycount.hpp"
#include "microsim-core/schedule_definition.hpp"
#include "microsim-core/schedule.hpp"
#include <stdexcept>

namespace averisera {
    namespace microsim {
        Contexts::Contexts()
        : _immutable(std::make_shared<ImmutableContext>()), _mutable(std::make_shared<MutableContext>())
        {}

		Contexts::Contexts(Contexts&& other)
			: _immutable(std::move(other._immutable)),
			_mutable(std::move(other._mutable)) {
			other._immutable = nullptr;
			other._mutable = nullptr;
		}

		Contexts& Contexts::operator=(Contexts&& other) {
			if (this != &other) {
				_mutable = std::move(other._mutable);
				_immutable = std::move(other._immutable);
				other._immutable = nullptr;
				other._mutable = nullptr;
			}
			return *this;
		}

        Contexts::Contexts(Date asof) {
            ScheduleDefinition def;
            def.start = asof;
            def.end = asof;
            def.frequency = Period(PeriodType::DAYS, 0);
            def.daycount = Daycount::YEAR_FRACT();
            Schedule schedule(def);
            _immutable = std::make_shared<ImmutableContext>(schedule, Ethnicity::IndexConversions());
            _mutable = std::make_shared<MutableContext>();
        }

        Contexts::Contexts(const Schedule& schedule) {
            _immutable = std::make_shared<ImmutableContext>(schedule, Ethnicity::IndexConversions());
            _mutable = std::make_shared<MutableContext>();
        }

		Contexts::Contexts(Schedule&& schedule) {
			_immutable = std::make_shared<ImmutableContext>(std::move(schedule), Ethnicity::IndexConversions());
			_mutable = std::make_shared<MutableContext>();
		}
        
        Contexts::Contexts(std::shared_ptr<ImmutableContext> immutable_ctx, std::shared_ptr<MutableContext> mutable_ctx)
        : _immutable(immutable_ctx), _mutable(mutable_ctx) {
            if (immutable_ctx == nullptr || mutable_ctx == nullptr) {
                throw std::domain_error("Contexts: Pointers cannot be null");
            }
        }
        
        /*Date Contexts::asof() const {
            assert(_mutable);
            assert(_immutable);
            const size_t idx = _mutable->period_index();
			return _immutable->schedule().date(idx);
        }*/
        
        size_t Contexts::asof_idx() const {
            assert(_mutable);
            return _mutable->date_index();
        }
        
        SchedulePeriod Contexts::current_period() const {
            assert(_mutable);
            assert(_immutable);
            const size_t idx = _mutable->date_index();
            if (idx < _immutable->schedule().size()) {
                return _immutable->schedule()[idx];
            } else {
                const Date d = asof();
                return SchedulePeriod(d);
            }
        }
    }
}
