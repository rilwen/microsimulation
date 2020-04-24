#include <gtest/gtest.h>
#include "microsim-calibrator/mortality_calibrator.hpp"
#include "microsim-core/anchored_hazard_curve.hpp"
#include "microsim-core/hazard_curve.hpp"
#include "microsim-core/mortality_rate.hpp"
#include "core/period.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(MortalityCalibrator, OneRate) {
	const MortalityCalibrator::age_group_type age_group(10, 21);
	const double r = 0.01;
	const std::vector<MortalityRate<MortalityCalibrator::age_group_type>> rates({ MortalityRate<MortalityCalibrator::age_group_type>(2000, r, age_group) });
	const MortalityCalibrator::year_t min_year_of_birth = 1940;
	const MortalityCalibrator::year_t max_year_of_birth = 2010;
	const std::vector<std::unique_ptr<AnchoredHazardCurve>> curves(MortalityCalibrator::calc_mortality_curves(rates, min_year_of_birth, max_year_of_birth));
	ASSERT_EQ(max_year_of_birth - min_year_of_birth + 1, curves.size());
	const double expected_hazard_rate = HazardCurve::integrated_hazard_rate_from_jump_proba(r) * 365.0 / 366.0; // because 2000 was a leap year
	for (size_t i = 0; i < curves.size(); ++i) {
		ASSERT_NE(nullptr, curves[i]) << i;
		ASSERT_EQ(Date(static_cast<Date::year_type>(min_year_of_birth + i), 1, 1), curves[i]->start()) << i;
		EXPECT_NEAR(expected_hazard_rate, curves[i]->average_hazard_rate(curves[i]->start(), curves[i]->start()), 1E-15) << curves[i]->start();
	}
}

typedef MortalityCalibrator::age_group_type age_group_type;
typedef MortalityRate<age_group_type> rate_type;

TEST(MortalityCalibrator, FewRates) {
    std::vector<rate_type> rates;
	rates.push_back(rate_type(2005, 0.015, age_group_type(4, 7)));
	rates.push_back(rate_type(1995, 0.03, age_group_type(60, 71)));
    rates.push_back(rate_type(2000, 0.01, age_group_type(10, 16)));
    const MortalityCalibrator::year_t min_year = 1920;
	const MortalityCalibrator::year_t max_year = 2010;
	const std::vector<std::unique_ptr<AnchoredHazardCurve>> curves(MortalityCalibrator::calc_mortality_curves(rates, min_year, max_year));
	ASSERT_EQ(max_year - min_year + 1, curves.size());
	for (size_t i = 0; i < curves.size(); ++i) {
		ASSERT_NE(nullptr, curves[i]) << i;
		ASSERT_EQ(Date(static_cast<Date::year_type>(min_year + i), 1, 1), curves[i]->start()) << i;
	}
	ASSERT_NEAR(0.03, curves[0]->conditional_jump_probability(Date(1995, 1, 1), Date(1996, 1, 1)), 1E-15);
	ASSERT_NEAR(0.03, curves[1]->conditional_jump_probability(Date(1995, 1, 1), Date(1996, 1, 1)), 1E-15);
    ASSERT_NEAR(0.03, curves[(1995 - 60) - min_year]->conditional_jump_probability(Date(1995, 1, 1), Date(1996, 1, 1)), 1E-15);
    ASSERT_NEAR(0.01, curves[(2000 - 10) - min_year]->conditional_jump_probability(Date(2000, 1, 1), Date(2001, 1, 1)), 1E-15);
	ASSERT_NEAR(0.01, curves[(2000 - 15) - min_year]->conditional_jump_probability(Date(2000, 1, 1), Date(2001, 1, 1)), 1E-15);
    ASSERT_NEAR(0.015, curves[(2005 - 4) - min_year]->conditional_jump_probability(Date(2005, 1, 1), Date(2006, 1, 1)), 1E-15);
	ASSERT_NEAR(0.015, curves[(2005 - 6) - min_year]->conditional_jump_probability(Date(2005, 1, 1), Date(2006, 1, 1)), 1E-15);    
	ASSERT_NEAR(0.015, curves[max_year - min_year]->conditional_jump_probability(Date(2030, 1, 1), Date(2031, 1, 1)), 1E-15);
}

TEST(MortalityCalibrator, Matrix) {
	const std::vector<int> years({ 1990, 1991, 1992, 2000 });
	const std::vector<age_group_type> age_groups({ age_group_type(0, 11), age_group_type(11, 21), age_group_type(21, 81), age_group_type(81, 101) });
	Eigen::MatrixXd rates(4, 4);
	for (size_t r = 0; r < 4; ++r) {
		for (size_t c = 0; c < 4; ++c) {
			rates(r, c) = 0.01 * static_cast<double>(1 + r) + 0.001 * static_cast<double>(c);
		}
	}
	const MortalityCalibrator::year_t min_year = 1960; // min year of birth
	const MortalityCalibrator::year_t max_year = 2010; // max year of birth
	DataFrame<age_group_type, int> data(rates, age_groups, years);
	const std::vector<std::unique_ptr<AnchoredHazardCurve>> curves(MortalityCalibrator::calc_mortality_curves(data, min_year, max_year));
	ASSERT_EQ(max_year - min_year + 1, curves.size());
	for (size_t i = 0; i < curves.size(); ++i) {
		ASSERT_NE(nullptr, curves[i]) << i;
		ASSERT_EQ(Date(static_cast<Date::year_type>(min_year + i), 1, 1), curves[i]->start()) << i;
	}
	ASSERT_NEAR(HazardCurve::integrated_hazard_rate_from_jump_proba(rates(3, 0)) * 365.0 / 366.0, curves.back()->average_hazard_rate(curves.back()->start(), curves.back()->start()), 1E-15);
	Period delta = Period::years(1); // 1Y period to automatically adjust to leap years
	for (size_t r = 0; r < 4; ++r) {
		const auto year = years[r];
		const double tol = 1e-15;
		for (size_t c = 0; c < 4; ++c) {
			const auto age_group = age_groups[c];
			const double expected_rate = rates(r, c);
			for (auto age = age_group.begin(); age < age_group.end(); ++age) {
				const auto year_of_birth = year - age;
				if (year_of_birth >= min_year && year_of_birth <= max_year) {
					const auto& curve = curves[year_of_birth - min_year];
					const Date d1(static_cast<Date::year_type>(year), 1, 1);
					const Date d2 = d1 + delta;
					const double actual_rate = curve->conditional_jump_probability(d1, d2);
					ASSERT_NEAR(expected_rate, actual_rate, tol) << "R=" << r << ", C=" << c << ", AGE=" << age << ", YOB=" << year_of_birth << ", YEAR=" << year << ", D1=" << d1 << ", D2=" << d2;
				}
			}
		}
	}
    // test the last curve
    const auto& last_curve = curves.back();
	delta = Period::days(366); // 2000 was a leap year
	for (size_t c = 0; c < 4; ++c) {
        const auto age_group = age_groups[c];
        const double expected_rate = rates(3, c);
        for (auto age = age_group.begin(); age < age_group.end(); ++age) {
            const auto yr = static_cast<Date::year_type>(max_year + age);
			const Date d1(yr, 1, 1);
			const Date d2 = d1 + delta;
            const double actual_rate = last_curve->conditional_jump_probability(d1, d2);
			ASSERT_NEAR(expected_rate, actual_rate, 1E-5) << c << " " << age << ", d1=" << d1 << ", d2=" << d2;
        }
    }
    // test the first curve
	delta = Period::days(365); // 1990 was not a leap year
    const auto& first_curve = curves.front();
    for (size_t c = 0; c < 4; ++c) {
        const auto age_group = age_groups[c];
        const double expected_rate = rates(0, c);
        for (auto age = age_group.begin(); age < age_group.end(); ++age) {
            const auto yr = static_cast<Date::year_type>(min_year + age);
            if (yr <= years.front()) {
				const Date d1(yr, 1, 1);
				const Date d2 = d1 + delta;
                const double actual_rate = first_curve->conditional_jump_probability(d1, d2);
				EXPECT_NEAR(expected_rate, actual_rate, 1E-15) << c << " " << age << ", d1=" << d1 << ", d2=" << d2;
            }
        }
    }
}
