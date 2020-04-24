#include <gtest/gtest.h>
#include "core/eigen.hpp"

TEST(Eigen, FromVec) {
    std::vector<double> v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    Eigen::Ref<Eigen::VectorXd> r = averisera::EigenUtils::from_vec(v);
    ASSERT_EQ(10, r.size());
    ASSERT_EQ(2.0, r[2]);
    ASSERT_EQ(&v[0], r.data());
    ASSERT_EQ(2.0, *(r.data() + 2));
    Eigen::Ref<Eigen::VectorXd> s = r.segment(3, 3);
    ASSERT_EQ(3, s.size());
    ASSERT_EQ(3.0, s[0]);
    ASSERT_EQ(&v[3], s.data());
    ASSERT_EQ(&v[4], s.data() + 1);
}

TEST(Eigen, Move) {
    Eigen::MatrixXd m1(2,2);
    m1 << 1, 2,
        3, 4;
    double* const p = m1.data();
    Eigen::MatrixXd m2(std::move(m1));
    m1.resize(0, 0);
    ASSERT_EQ(p, m2.data());
    ASSERT_EQ(m2.rows(), 2);
    ASSERT_EQ(m2.cols(), 2);
    ASSERT_EQ(1, m2(0, 0));
    ASSERT_EQ(4, m2(1, 1));
}

TEST(Eigen, LayoutAssumption) {
	Eigen::MatrixXd m(2, 2);
	m(0, 0) = 1;
	m(1, 0) = 2;
	m(0, 1) = 3;
	m(1, 1) = 4;
	const double* data = m.data();
	ASSERT_EQ(1, data[0]);
	ASSERT_EQ(2, data[1]);
	ASSERT_EQ(3, data[2]);
	ASSERT_EQ(4, data[3]);
}