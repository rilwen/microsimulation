// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "core/sacado_eigen.hpp"
#include "core/statistics.hpp"

template <class Scalar>
Scalar fixed_func(const Scalar& a, const Scalar& b, const Scalar& c) {
	return a * std::exp(b * b * c);
}

template <class Scalar>
Scalar fixed_func_da(const Scalar& /*a*/, const Scalar& b, const Scalar& c) {
	return std::exp(b * b * c);
}

template <class Scalar>
Scalar fixed_func_db(const Scalar& a, const Scalar& b, const Scalar& c) {
	return 2 * a * b * c * std::exp(b * b * c);
}

TEST(Sacado, FixedNumberStatic) {
	double a = 1.2;
	double b = 0.4;
	double c = -2.1;
	const unsigned int dim = 2;
	Sacado::Fad::SFad<double, dim> afad(dim, 0, a);
	Sacado::Fad::SFad<double, dim> bfad(dim, 1, b);
	Sacado::Fad::SFad<double, dim> cfad(c);	
	
	const double r = fixed_func(a, b, c);

	Sacado::Fad::SFad<double, dim> rfad = fixed_func(afad, bfad, cfad);

	ASSERT_EQ(r, rfad.val());
	ASSERT_EQ(fixed_func_da(a, b, c), rfad.dx(0));
	ASSERT_EQ(fixed_func_db(a, b, c), rfad.dx(1));
}

TEST(Sacado, FixedNumberDynamic) {
	double a = 1.2;
	double b = 0.4;
	double c = -2.1;
	const unsigned int dim = 2;
	averisera::adouble afad(dim, 0, a);
	averisera::adouble bfad(dim, 1, b);
	averisera::adouble cfad(c);

	const double r = fixed_func(a, b, c);

	averisera::adouble rfad = fixed_func(afad, bfad, cfad);

	ASSERT_EQ(r, rfad.val());
	ASSERT_EQ(fixed_func_da(a, b, c), rfad.dx(0));
	ASSERT_EQ(fixed_func_db(a, b, c), rfad.dx(1));
}

TEST(Sacado, VariableDim) {
	const unsigned int dim = 4;
	std::vector<averisera::adouble> inputs(dim);
	std::vector<double> params = { 0.1, 0.2, -0.1, 0.3 };
	inputs[0] = averisera::adouble(dim, 0, 0.11);
	inputs[1] = averisera::adouble(dim, 1, 0.19);
	inputs[2] = averisera::adouble(dim, 2, -0.08);
	inputs[3] = averisera::adouble(dim, 3, 0.32);
	averisera::adouble result = 0.0;
	double result_dbl = 0.0;
	for (unsigned int i = 0; i < dim; ++i) {
		averisera::adouble tmp = inputs[i] - params[i];
		tmp = std::pow(tmp, 2);
		result += tmp;
		result_dbl += std::pow(inputs[i].val() - params[i], 2);
	}
	ASSERT_EQ(result_dbl, result);
	for (unsigned int i = 0; i < dim; ++i) {
		ASSERT_NEAR(2 * (inputs[i].val() - params[i]), result.dx(i), 1E-17) << i;
	}
}

//TEST(Sacado, Eigen) {
//	const unsigned int dim = 4;
//	Eigen::VectorXa inputs(dim);
//	Eigen::VectorXa params(dim);
//	params << 0.1, 0.2, -0.1, 0.3;
//	inputs[0] = averisera::adouble(dim, 0, 0.11);
//	inputs[1] = averisera::adouble(dim, 1, 0.19);
//	inputs[2] = averisera::adouble(dim, 2, -0.08);
//	inputs[3] = averisera::adouble(dim, 3, 0.32);
//	averisera::adouble result = (inputs - params).squaredNorm(); // This really computes the gradient as well!
//	double result_dbl = 0.0;
//	for (unsigned int i = 0; i < dim; ++i) {
//		result_dbl += std::pow(inputs[i].val() - params[i].val(), 2);
//	}
//	ASSERT_EQ(result_dbl, result);
//	for (unsigned int i = 0; i < dim; ++i) {
//		ASSERT_NEAR(2 * (inputs[i].val() - params[i].val()), result.dx(i), 1E-17) << i;
//	}
//
//	//Square root too
//	result = (inputs - params).norm(); // This really computes the gradient as well!
//	const double sqrt_result_dbl = sqrt(result_dbl);
//	ASSERT_EQ(sqrt_result_dbl, result);
//	for (unsigned int i = 0; i < dim; ++i) {
//		ASSERT_NEAR((inputs[i].val() - params[i].val()) / sqrt_result_dbl, result.dx(i), 1E-17) << i;
//	}
//
//	Eigen::MatrixXa A(dim, dim);
//	A.row(0) = inputs;
//	Eigen::MatrixXa B(A);
//	A.col(0) = B.col(0) + B.col(1) / 0.2;
//	A = B;
//	ASSERT_EQ(A, B);	
//#ifdef _WIN64
//	Eigen::MatrixXa C = A * B;
//	Eigen::MatrixXd p1(2, 2);
//	Eigen::MatrixXd p2;
//	p2 = p1;
//#endif
//}

TEST(Sacado, KullbackLeibler) {
	const unsigned int dim = 4;
	std::vector<averisera::adouble> p = { 0.1, 0.2, 0.3, 0.4 };
	std::vector<averisera::adouble> q(dim);
	
	for (unsigned int i = 0; i < dim; ++i) {
		q[i] = averisera::adouble(dim, i, p[i].val());
	}

	const averisera::adouble divergence = averisera::Statistics::kl_divergence<std::vector<averisera::adouble>, std::vector<averisera::adouble>, averisera::adouble>(p, q);
	ASSERT_NEAR(0.0, divergence.val(), 1E-17);
	for (unsigned int i = 0; i < dim; ++i) {
		ASSERT_NEAR(-1, divergence.dx(i), 1E-15) << i;
	}
}

TEST(Sacado, SecondDerivative) {	
	typedef averisera::adouble adouble;
	typedef Sacado::Fad::DFad<adouble> aadouble;
	const unsigned int dim = 2;
	const double v[] = { 1.0, 2.0 };
	aadouble x[dim];
	for (unsigned int i = 0; i < dim; ++i) {
		x[i] = averisera::NestedADouble<1>::from_double(dim, i, v[i]);
		ASSERT_EQ(v[i], x[i].val().val()) << i;
		for (unsigned int j = 0; j < dim; ++j) {
			const double dxi_dxj = i == j ? 1.0 : 0.0;
			ASSERT_EQ(dxi_dxj, x[i].dx(j).val()) << i << " " << j;
			for (unsigned int k = 0; k < dim; ++k) {
				ASSERT_EQ(0., x[i].dx(j).dx(k)) << i << " " << j << " " << k;
			}
		}
	}
	aadouble z = x[0] * x[1];
	ASSERT_EQ(v[0] * v[1], z.val().val());
	ASSERT_EQ(1.0, z.dx(1).dx(0));
	ASSERT_EQ(z.dx(0).dx(1), z.dx(1).dx(0));
	z = x[1] * x[1];
	ASSERT_EQ(2 * v[1], z.dx(1).val());
	ASSERT_EQ(2., z.dx(1).dx(1));	
}
