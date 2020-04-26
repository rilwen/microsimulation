// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/hazard_rate_multiplier_provider/hazard_rate_multiplier_provider_bypred_timedependent.hpp"

using namespace averisera;
using namespace averisera::microsim;

class PredDouble : public Predicate<double> {
public:
	PredDouble(double from, double to)
		: from_(from), to_(to) {}

	bool select_out_of_context(const double& x) const override {
		return x >= from_ && x < to_;
	}

	bool select(const double& x, const Contexts&) const override {
		return select_out_of_context(x);
	}

	PredDouble* clone() const override {
		return new PredDouble(from_, to_);
	}

	void print(std::ostream& os) const override {
		os << "Double(" << from_ << ", " << to_ << ")";
	}
private:
	double from_;
	double to_;
};

TEST(HazardRateMultiplierProviderByPredTimeDependent, Test) {
	Contexts ctx(Date(2010, 5, 1));
	std::vector<HazardRateMultiplierProviderByPredTimeDependent<double>::pred_hrm_ser_pair> pairs;
	TimeSeries<Date, HazardRateMultiplier> s1;
	TimeSeries<Date, HazardRateMultiplier> s2;
	const HazardRateMultiplier h1(0.5);
	const HazardRateMultiplier h2(1.4);
	const HazardRateMultiplier h3(0.99);
	s1.push_back(Date(1995, 1, 1), h1);
	s1.push_back(Date(2015, 1, 1), h2);
	s2.push_back(Date(2000, 1, 1), h3);
	pairs.push_back(std::make_pair(std::make_unique<PredDouble>(-100, 0), s1));
	pairs.push_back(std::make_pair(std::make_unique<PredDouble>(1, 10), s2));
	HazardRateMultiplierProviderByPredTimeDependent<double> provider(std::move(pairs));
	ASSERT_EQ(h1, provider(-2, ctx));
	ASSERT_EQ(h2, provider(-2, Contexts(Date(2020, 4, 4))));
	ASSERT_EQ(h3, provider(2, ctx));
	ASSERT_EQ(HazardRateMultiplier(), provider(0.4, ctx));
}
