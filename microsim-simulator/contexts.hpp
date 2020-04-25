/*
(C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_MS_CONTEXTS_H
#define __AVERISERA_MS_CONTEXTS_H

#include "mutable_context.hpp"
#include "core/dates_fwd.hpp"
#include "microsim-core/schedule.hpp"
#include <cassert>
#include <memory>

namespace averisera {
    namespace microsim {
        class ImmutableContext;
        class MutableContext;
        
        /** @brief Holds pointers to ImmutableContext and MutableContext. */
        class Contexts {
        public:
            /** Default context */
            Contexts();

			/** Move constructor */
			Contexts(Contexts&& other);

			/** Move assignment */
			Contexts& operator=(Contexts&& other);

            Contexts(const Contexts& other) = delete;
			Contexts& operator=(const Contexts& other) = delete;

            /** Contexts with single simulation date */
            Contexts(Date asof);

            /** Contexts with a simulation schedule */
            Contexts(const Schedule& schedule);
			Contexts(Schedule&& schedule);
            
            /** Construct object with pointers to contexts.
             * 
             * @param[in] immutable_ctx Pointer to instance of ImmutableContext
             * @param[in] mutable_ctx Pointer to instance of MutableContext
             * @throw std::domain_error If either of the pointers is null.
             */
            Contexts(std::shared_ptr<ImmutableContext> immutable_ctx, std::shared_ptr<MutableContext> mutable_ctx);
            
            const ImmutableContext& immutable_ctx() const {
                assert(_immutable);
                return *_immutable;
            }

            ImmutableContext& immutable_ctx() {
                assert(_immutable);
                return *_immutable;
            }
            
            MutableContext& mutable_ctx() const {
                assert(_mutable);
                return *_mutable;
            }
            
            /** Get asof date (start of current schedule period) */
			Date asof() const {
				assert(_mutable);
				assert(_immutable);
				return _mutable->asof(*_immutable);
			}
            
            /** Get index of asof() date */
            size_t asof_idx() const;
            
            /** Get current schedule period. */
            SchedulePeriod current_period() const;
        private:
            std::shared_ptr<ImmutableContext> _immutable;
            std::shared_ptr<MutableContext> _mutable;
        };
    }
}

#endif // __AVERISERA_MS_CONTEXTS_H
