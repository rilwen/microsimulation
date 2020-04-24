#include <gtest/gtest.h>
#include "microsim-core/conception.hpp"
#include "microsim-core/hazard_model.hpp"
#include "microsim-core/hazard_curve_factory.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "core/daycount.hpp"
#include "core/generic_distribution_integral.hpp"
#include "core/period.hpp"
#include "core/time_series.hpp"

using namespace averisera;
using namespace averisera::microsim;

static const auto DAYCOUNT = Daycount::YEAR_FRACT();
static const auto FACTORY = HazardCurveFactory::PIECEWISE_CONSTANT();

TEST(Conception, Test) {
    const Date conception_curve_start(2010, 1, 1);
    std::shared_ptr<const AnchoredHazardCurve> conception_curve = AnchoredHazardCurve::build(conception_curve_start, DAYCOUNT, FACTORY, std::vector<Date>({conception_curve_start + Period::years(1)}), std::vector<double>({0.01}), std::vector<HazardRateMultiplier>());
    // numbers are made up
    const auto md0 = std::make_shared<GenericDistributionIntegral<Conception::multiplicity_type>>(1, std::vector<double>({0.99, 0.01}));
    const auto md35 = std::make_shared<GenericDistributionIntegral<Conception::multiplicity_type>>(1, std::vector<double>({0.96, 0.04}));
    Conception::mdistr_series_type multiplicity_distros;
    multiplicity_distros.push_back(0., md0);
    multiplicity_distros.push_back(35., md35);
	Conception::mdistr_multi_series_type md_serii;
	md_serii.push_back(2010, multiplicity_distros);
    Conception conception(conception_curve, std::move(md_serii));
    ASSERT_EQ(2u, conception.hazard_model(Date(2015, 10, 4)).dim());
}
