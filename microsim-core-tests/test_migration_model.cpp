#include <gtest/gtest.h>
#include "microsim-core/migration_model.hpp"

using namespace averisera;
using namespace averisera::microsim;

TEST(MigrationModel, CalculateMigration) {
	const MigrationModel::MigrationRatePerAnnum r1(0, 2500);
	ASSERT_NEAR(5000.0, MigrationModel::calculate_migration(10000.0, 2.0, r1), 1E-12);
	const MigrationModel::MigrationRatePerAnnum r2(-0.01, 1);
	ASSERT_NEAR(100.0 - 1000.0, MigrationModel::calculate_migration(1000.0, 1E20, r2), 1E-12);
	ASSERT_NEAR(100.0 - 10.0, MigrationModel::calculate_migration(10.0, 1E20, r2), 1E-12);
	ASSERT_NEAR((1 - 1000.0 * 0.01) * 1e-8, MigrationModel::calculate_migration(1000.0, 1e-8, r2), 1E-15);
	ASSERT_NEAR((1 - 10.0 * 0.01) * 1e-8, MigrationModel::calculate_migration(10.0, 1e-8, r2), 1E-15);
}

TEST(MigrationModel, CalibrateRate) {
	const double x0 = 10000;
	const double t = 0.5;
	MigrationModel::MigrationRatePerAnnum r(0, 2500);
	double xt = x0 + MigrationModel::calculate_migration(x0, t, r);
	MigrationModel::MigrationRatePerAnnum r2 = MigrationModel::calibrate_rate(x0, xt, t, r.absolute());
	ASSERT_NEAR(r.relative(), r2.relative(), 1e-8);
	r = MigrationModel::MigrationRatePerAnnum(0.04, 1000);
	xt = x0 + MigrationModel::calculate_migration(x0, t, r);
	r2 = MigrationModel::calibrate_rate(x0, xt, t, r.absolute());
	ASSERT_NEAR(r.relative(), r2.relative(), 1e-8);
	r = MigrationModel::MigrationRatePerAnnum(-0.01, 1000);
	xt = x0 + MigrationModel::calculate_migration(x0, t, r);
	r2 = MigrationModel::calibrate_rate(x0, xt, t, r.absolute());
	ASSERT_NEAR(r.relative(), r2.relative(), 1e-8);
	r = MigrationModel::MigrationRatePerAnnum(0.04, 0);
	xt = x0 + MigrationModel::calculate_migration(x0, t, r);
	r2 = MigrationModel::calibrate_rate(x0, xt, t, r.absolute());
	ASSERT_NEAR(r.relative(), r2.relative(), 1e-8);
	r = MigrationModel::MigrationRatePerAnnum(-0.01, 0);
	xt = x0 + MigrationModel::calculate_migration(x0, t, r);
	r2 = MigrationModel::calibrate_rate(x0, xt, t, r.absolute());
	ASSERT_NEAR(r.relative(), r2.relative(), 1e-8);
	r = MigrationModel::MigrationRatePerAnnum(-0.01, -0.001);
	xt = x0 + MigrationModel::calculate_migration(x0, t, r);
	r2 = MigrationModel::calibrate_rate(x0, xt, t, r.absolute());
	ASSERT_NEAR(r.relative(), r2.relative(), 1e-8);
}
