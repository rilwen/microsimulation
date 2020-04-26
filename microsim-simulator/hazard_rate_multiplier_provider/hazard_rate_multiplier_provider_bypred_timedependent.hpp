// (C) Averisera Ltd 2014-2020
#pragma once
#include "../contexts.hpp"
#include "../feature.hpp"
#include "../hazard_rate_multiplier_provider.hpp"
#include "../predicate.hpp"
#include "microsim-core/hazard_rate_multiplier.hpp"
#include "core/preconditions.hpp"
#include "core/time_series.hpp"
#include <algorithm>
#include <utility>
#include <vector>

namespace averisera {
	namespace microsim {
		/** HazardRateMultiplierProvider which matches A with a vector of predicates and returns a time-dependent HazardRateMultiplier for the first Predicate which selects A. Time-dependent HazardRateMultipliers are stored in TimeSeries indexed by Date of simulation. If no Predicate selects A, return default multiplier of 1.0. Predicates are checked in the order they are provided to the constructor. */
		template <class A> class HazardRateMultiplierProviderByPredTimeDependent: public HazardRateMultiplierProvider<A> {
		public:
			typedef TimeSeries<Date, HazardRateMultiplier> hrm_series;
			typedef std::pair<std::unique_ptr<const Predicate<A>>, hrm_series> pred_hrm_ser_pair;

			/** @throw std::domain_error If any Predicate is null or TimeSeries is empty */
			HazardRateMultiplierProviderByPredTimeDependent(std::vector<pred_hrm_ser_pair>&& pairs) {
				check_that(std::all_of(pairs.begin(), pairs.end(), [](const pred_hrm_ser_pair& p) {
					return (p.first != nullptr) && (!p.second.empty());
				}), "HazardRateMultiplierProviderByPredTimeDependent: null Predicate or empty TimeSeries");
				pairs_ = std::move(pairs);
			}

			HazardRateMultiplier operator()(const A& arg, const Contexts& ctx) const override {
				for (const auto& p : pairs_) {
					if (p.first->select(arg, ctx)) {
						return p.second.padded_value(ctx.asof());
					}
				}
				return HazardRateMultiplier();
			}

			const FeatureUser<Feature>::feature_set_t& requires() const override {
				return Feature::empty();
			}

			std::unique_ptr<HazardRateMultiplierProvider<A>> clone() const override {
				std::vector<pred_hrm_ser_pair> pairs_copy;
				pairs_copy.reserve(pairs_.size());
				for (const auto& p : pairs_) {
					pairs_copy.push_back(std::make_pair(std::unique_ptr<const Predicate<A>>(p.first->clone()), p.second));
				}
				return std::make_unique<HazardRateMultiplierProviderByPredTimeDependent>(std::move(pairs_copy));
			}
		private:
			std::vector<pred_hrm_ser_pair> pairs_;			
		};
	}
}
