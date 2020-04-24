#include <gtest/gtest.h>
#include "core/generic_distribution_from_real.hpp"
#include "core/discrete_distribution.hpp"
#include "core/normal_distribution.hpp"
#include <cmath>
#include <limits>

using namespace averisera;

static const double FACTOR = 2;

class MockGenericDistributionFromReal: public GenericDistributionFromReal<double> {
public:
    MockGenericDistributionFromReal(std::shared_ptr<const Distribution> real_distr)
        : GenericDistributionFromReal<double>(real_distr) {
    }

    MockGenericDistributionFromReal* conditional(double left, double right) const override {
        return new MockGenericDistributionFromReal(conditional_real_distr(left, right));
    }
private:
    double to_double(double value) const override {
        return value * FACTOR;
    }
    
    double from_double(double value) const override {
        return value / FACTOR;
    }
};

TEST(GenericDistributionFromReal, FromNormal) {
    const auto normal = std::make_shared<NormalDistribution>();
    MockGenericDistributionFromReal mock(normal);
    ASSERT_EQ(mock.lower_bound(), - std::numeric_limits<double>::infinity());
    ASSERT_EQ(mock.upper_bound(), std::numeric_limits<double>::infinity());
    ASSERT_EQ(0.0, mock.icdf_generic(0.5));
    ASSERT_NEAR(mock.icdf_generic(0.8) * FACTOR, normal->icdf(0.8), 1E-14);
    ASSERT_NEAR(mock.range_prob2(-1 / FACTOR, 2 / FACTOR), normal->range_prob2(-1, 2), 1E-15);
}

TEST(GenericDistributionFromReal, FromDiscrete) {
    const int a = -1;
    const std::vector<double> probs({0.19, 0.52, 0.29}); 
    MockGenericDistributionFromReal mock(std::make_shared<DiscreteDistribution>(a, probs));
    ASSERT_EQ(mock.lower_bound(), static_cast<double>(a) / FACTOR);
    ASSERT_EQ(mock.upper_bound(), static_cast<double>(a + probs.size() - 1) / FACTOR);
    ASSERT_NEAR(-1/FACTOR, mock.icdf_generic(probs[0]-1E-12), 1E-15);
    ASSERT_NEAR(1/FACTOR, mock.icdf_generic(1.0), 1E-15);
}
