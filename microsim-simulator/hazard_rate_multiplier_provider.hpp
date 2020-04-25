#pragma once
#include "functor.hpp"

namespace averisera {
	namespace microsim {
		struct HazardRateMultiplier;

		template <class A> class HazardRateMultiplierProvider : public Functor<A, HazardRateMultiplier> {
		public:
			using Functor<A, HazardRateMultiplier>::operator();

			/** Deep copy */
			virtual std::unique_ptr<HazardRateMultiplierProvider> clone() const = 0;
		};
	}
}
