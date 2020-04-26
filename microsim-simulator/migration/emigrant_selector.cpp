// (C) Averisera Ltd 2014-2020
#include "emigrant_selector.hpp"
#include "../contexts.hpp"
#include "../mutable_context.hpp"
#include "../predicate.hpp"
#include "../predicate_factory.hpp"
#include "core/preconditions.hpp"

namespace averisera {
	namespace microsim {
		EmigrantSelector::EmigrantSelector(std::shared_ptr<const Predicate<Person>> predicate, Date from, Date to, unsigned int min_age)
			: from_(from), to_(to), min_age_(min_age) {
			check_not_null(predicate, "EmigrantSelector: null predicate");
			if (min_age > 0) {
				predicate_ = predicate->product(PredicateFactory::make_min_age(min_age, true));
			} else {
				predicate_ = predicate->product(PredicateFactory::make_alive());
			}
		}
		void EmigrantSelector::select(const Contexts& ctx, const Date asof, std::unordered_map<Date, std::vector<std::shared_ptr<Person>>>& selected) const {
			for (auto& kv : selected) {
				kv.second.clear();
			}
			if (predicate_->active(asof)) {
				for (const auto& kv : ctx.mutable_ctx().emigrants()) {
					if (kv.first >= from_ && kv.first < to_) {
						predicate_->select<Person>(kv.second, ctx, selected[kv.first]);						
					}
				}				
			}
		}

		void EmigrantSelector::select(const Contexts& ctx, Date asof, std::vector<std::shared_ptr<Person>>& selected) const {
			selected.clear();
			if (predicate_->active(asof)) {
				for (const auto& kv : ctx.mutable_ctx().emigrants()) {
					if (kv.first >= from_ && kv.first < to_) {
						predicate_->select<Person>(kv.second, ctx, selected);
					}
				}
			}
		}

		std::string EmigrantSelector::predicate_as_string() const {
			return predicate_->as_string();
		}
	}
}
