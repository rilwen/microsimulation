#include <gtest/gtest.h>
#include "core/period.hpp"
#include "microsim-simulator/contexts.hpp"
#include "microsim-simulator/initialiser/perturb_date_of_birth_day.hpp"
#include "microsim-simulator/initialiser/perturb_date_of_birth_month.hpp"
#include "microsim-simulator/initialiser/perturb_history_values_double.hpp"
#include "microsim-simulator/person_data.hpp"
#include "helpers.hpp"

using namespace averisera;
using namespace averisera::microsim;


TEST(Perturbations, PerturbDateOfBirthDayNoShift) {
    const Date d1(1989, 6, 4);
    const Date d2(1988, 9, 6);
    const Date d3(2010, 4, 5);
    const Date d4(1994, 11, 3);
    std::vector<PersonData> datas(2);
    datas[0].id = 1;
    datas[0].date_of_birth = d1;
    datas[0].conception_date = d2;
    datas[0].histories["BMI"] = HistoryData("BMI", HistoryData::type_t::DOUBLE);
    datas[0].histories["BMI"].append(d3, 23.0);
    datas[1].id = 2;
    datas[1].mother_id = 1;
    datas[1].date_of_birth = d4;
    Contexts ctx(ctx_with_rng_precalc({0.4}));
    PerturbDateOfBirthDay p1(false);
    p1.apply(datas, ctx);
    ASSERT_NE(d1, datas[0].date_of_birth);
    ASSERT_EQ(d1.month(), datas[0].date_of_birth.month());
    ASSERT_EQ(d1.year(), datas[0].date_of_birth.year());
    ASSERT_EQ(d1 - d2, datas[0].date_of_birth - datas[0].conception_date);
    ASSERT_EQ(d3, datas[0].histories["BMI"].dates()[0]);
    ASSERT_EQ(d4, datas[1].date_of_birth);
}


TEST(Perturbations, PerturbDateOfBirthDayWithShift) {
    const Date d1(1989, 6, 4);
    const Date d2(1988, 9, 6);
    const Date d3(2010, 4, 5);
    const Date d4(1994, 11, 3);
    std::vector<PersonData> datas(2);
    datas[0].id = 1;
    datas[0].date_of_birth = d1;
    datas[0].conception_date = d2;
    datas[0].histories["BMI"] = HistoryData("BMI", HistoryData::type_t::DOUBLE);
    datas[0].histories["BMI"].append(d3, 23.0);
    datas[1].id = 2;
    datas[1].mother_id = 1;
    datas[1].date_of_birth = d4;
    Contexts ctx(ctx_with_rng_precalc({0.4}));
    PerturbDateOfBirthDay p1(true);
    p1.apply(datas, ctx);
    ASSERT_NE(d1, datas[0].date_of_birth);
    ASSERT_EQ(d1.month(), datas[0].date_of_birth.month());
    ASSERT_EQ(d1.year(), datas[0].date_of_birth.year());
    ASSERT_EQ(d1 - d2, datas[0].date_of_birth - datas[0].conception_date);
    ASSERT_EQ(d3 + (datas[0].date_of_birth - d1), datas[0].histories["BMI"].dates()[0]);
    ASSERT_EQ(d4, datas[1].date_of_birth);
}

TEST(Perturbations, PerturbDateOfBirthMonthNoShiftWithTrunc) {
    const Date d1(1989, 6, 30);
    const Date d2(1988, 9, 27);
    const Date d3(2010, 4, 5);
    const Date d4(1994, 11, 3);
    std::vector<PersonData> datas(2);
    datas[0].id = 1;
    datas[0].date_of_birth = d1;
    datas[0].conception_date = d2;
    datas[0].histories["BMI"] = HistoryData("BMI", HistoryData::type_t::DOUBLE);
    datas[0].histories["BMI"].append(d3, 23.0);
    datas[1].id = 2;
    datas[1].mother_id = 1;
    datas[1].date_of_birth = d4;
    Contexts ctx(ctx_with_rng_precalc({0.15}));
    PerturbDateOfBirthMonth p1(false);
    p1.apply(datas, ctx);
    ASSERT_NE(d1, datas[0].date_of_birth);
    ASSERT_EQ(2, datas[0].date_of_birth.month()); // We want to get February to test whether the code truncates 30th to 28th
    ASSERT_EQ(28, datas[0].date_of_birth.day());
    ASSERT_EQ(d1.year(), datas[0].date_of_birth.year());
    ASSERT_EQ(d1 - d2, datas[0].date_of_birth - datas[0].conception_date);
    ASSERT_EQ(d3, datas[0].histories["BMI"].dates()[0]);
    ASSERT_EQ(d4, datas[1].date_of_birth);
}


TEST(Perturbations, PerturbDateOfBirthMonthWithShiftWithTrunc) {
    const Date d1(1989, 6, 30);
    const Date d2(1988, 9, 27);
    const Date d3(2010, 4, 5);
    const Date d4(1994, 11, 3);
    std::vector<PersonData> datas(2);
    datas[0].id = 1;
    datas[0].date_of_birth = d1;
    datas[0].conception_date = d2;
    datas[0].histories["BMI"] = HistoryData("BMI", HistoryData::type_t::DOUBLE);
    datas[0].histories["BMI"].append(d3, 23.0);
    datas[1].id = 2;
    datas[1].mother_id = 1;
    datas[1].date_of_birth = d4;
    Contexts ctx(ctx_with_rng_precalc({0.15}));
    PerturbDateOfBirthMonth p1(true);
    p1.apply(datas, ctx);
    ASSERT_NE(d1, datas[0].date_of_birth);
    ASSERT_EQ(2, datas[0].date_of_birth.month()); // We want to get February to test whether the code truncates 30th to 28th
    ASSERT_EQ(28, datas[0].date_of_birth.day());
    ASSERT_EQ(d1.year(), datas[0].date_of_birth.year());
    ASSERT_EQ(d1 - d2, datas[0].date_of_birth - datas[0].conception_date);
    ASSERT_EQ(d3 + (datas[0].date_of_birth - d1), datas[0].histories["BMI"].dates()[0]);
    ASSERT_EQ(d4, datas[1].date_of_birth);
}


TEST(Perturbations, PerturbDateOfBirthMonthNoShiftNoTrunc) {
    const Date d1(1989, 6, 4);
    const Date d2(1988, 9, 6);
    const Date d3(2010, 4, 5);
    const Date d4(1994, 11, 3);
    std::vector<PersonData> datas(2);
    datas[0].id = 1;
    datas[0].date_of_birth = d1;
    datas[0].conception_date = d2;
    datas[0].histories["BMI"] = HistoryData("BMI", HistoryData::type_t::DOUBLE);
    datas[0].histories["BMI"].append(d3, 23.0);
    datas[1].id = 2;
    datas[1].mother_id = 1;
    datas[1].date_of_birth = d4;
    Contexts ctx(ctx_with_rng_precalc({0.15}));
    PerturbDateOfBirthMonth p1(false);
    p1.apply(datas, ctx);
    ASSERT_NE(d1, datas[0].date_of_birth);
    ASSERT_EQ(d1.day(), datas[0].date_of_birth.day());
    ASSERT_EQ(d1.year(), datas[0].date_of_birth.year());
    ASSERT_EQ(d1 - d2, datas[0].date_of_birth - datas[0].conception_date);
    ASSERT_EQ(d3, datas[0].histories["BMI"].dates()[0]);
    ASSERT_EQ(d4, datas[1].date_of_birth);
}


TEST(Perturbations, PerturbDateOfBirthMonthWithShiftNoTrunc) {
    const Date d1(1989, 6, 4);
    const Date d2(1988, 9, 6);
    const Date d3(2010, 4, 5);
    const Date d4(1994, 11, 3);
    std::vector<PersonData> datas(2);
    datas[0].id = 1;
    datas[0].date_of_birth = d1;
    datas[0].conception_date = d2;
    datas[0].histories["BMI"] = HistoryData("BMI", HistoryData::type_t::DOUBLE);
    datas[0].histories["BMI"].append(d3, 23.0);
    datas[1].id = 2;
    datas[1].mother_id = 1;
    datas[1].date_of_birth = d4;
    Contexts ctx(ctx_with_rng_precalc({0.15}));
    PerturbDateOfBirthMonth p1(true);
    p1.apply(datas, ctx);
    ASSERT_NE(d1, datas[0].date_of_birth);
    ASSERT_EQ(d1.day(), datas[0].date_of_birth.day());
    ASSERT_EQ(d1.year(), datas[0].date_of_birth.year());
    ASSERT_EQ(d1 - d2, datas[0].date_of_birth - datas[0].conception_date);
    ASSERT_EQ(d3 + (datas[0].date_of_birth - d1), datas[0].histories["BMI"].dates()[0]);
    ASSERT_EQ(d4, datas[1].date_of_birth);
}
