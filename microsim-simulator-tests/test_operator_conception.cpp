#include <gtest/gtest.h>
#include "core/daycount.hpp"
#include "core/generic_distribution_integral.hpp"
#include "core/period.hpp"
#include "core/time_series.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/hazard_curve_factory.hpp"
#include "microsim-simulator/operator/operator_conception.hpp"

using namespace averisera;
using namespace averisera::microsim;

static const auto DAYCOUNT = Daycount::YEAR_FRACT();
static const auto FACTORY = HazardCurveFactory::PIECEWISE_CONSTANT();


TEST(OperatorConception, HistoryGenerator) {
    const Date conception_curve_start(2010, 1, 1);
    std::shared_ptr<const AnchoredHazardCurve> conception_curve = AnchoredHazardCurve::build(conception_curve_start, DAYCOUNT, FACTORY, std::vector<Date>({conception_curve_start + Period::years(1)}), std::vector<double>({0.01}), std::vector<HazardRateMultiplier>());
    // numbers are made up
    const auto md0 = std::make_shared<GenericDistributionIntegral<Conception::multiplicity_type>>(1, std::vector<double>({0.99, 0.01}));
    const auto md35 = std::make_shared<GenericDistributionIntegral<Conception::multiplicity_type>>(1, std::vector<double>({0.96, 0.04}));
    Conception::mdistr_series_type multiplicity_distros;
    multiplicity_distros.push_back(0., md0);
    multiplicity_distros.push_back(35., md35);
	Conception::mdistr_multi_series_type mds;
	mds.push_back(2010, multiplicity_distros);
    Conception conception(conception_curve, std::move(mds));
    const auto opcon = std::make_shared<OperatorConception>(conception, nullptr, nullptr, 0, 1000, Period::months(0));
    ASSERT_EQ(1u, opcon->requirements().size());
}
