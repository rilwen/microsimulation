/*
(C) Averisera Ltd 2014-2020
*/
#ifndef __AVERISERA_ADAPT_1D_H
#define __AVERISERA_ADAPT_1D_H

#include <stdexcept>
#include <utility>
#include <cassert>
#include <cmath>

namespace averisera {

	struct Normalizator1D
	{
		double operator()(double x) const
		{
			return fabs(x);
		}
	};

	/** 
	1D adaptive quadrature integration algorithm.
	
	@tparam N Size of the GK integrator used (N,2*N+1)
	*/
	template <size_t N>
	class Adapt1D
	{
	public:
		/**
		@param[in] max_subdiv Maximum number of subdivisions
		@param[in] tolerance Tolerance for absolute error
		@param[in] blow_up When asked to return the error with the result, either throw runtime_error on failure to converge (true), or return the result anyway (default: false).
		*/
		Adapt1D(unsigned int max_subdiv, double tolerance, bool blow_up = false);

		/**
		Integrate a function.
		
		@param function Integrated 1D function with V(*function)(double) signature.
		@param x0 Lower bound of the integral.
		@param x1 Upper bound of the integral.
		@param[out] integral Value of the integral.
		@param normalizator Provides the operator()(const V& v) function which measures the norm of a result type.
		@tparam V Value type of the integral (e.g. double).		
		@throw std::runtime_error If could not converge.
		*/
		template <typename F, typename V, typename Norm>
		void integrate(F function, double x0, double x1, V& integral, Norm normalizator) const;
		
		/**
		Integrate a function with result type double.
		
		@param function Integrated 1D function with V(*function)(double) signature.
		@param x0 Lower bound of the integral.
		@param x1 Upper bound of the integral.
		@return Value of the integral.
		@throw std::runtime_error If could not converge.
		*/
		template <typename F>
		inline double integrate(F function, double x0, double x1) const;	

		/**
		Integrate a function, returning an error estimate.
		
		@param function Integrated 1D function with V(*function)(double) signature.
		@param x0 Lower bound of the integral.
		@param x1 Upper bound of the integral.
		@param[out] integral Value of the integral.
		@param normalizator Provides the operator()(const V& v) function which measures the norm of a result type.
		@tparam V Value type of the integral (e.g. double).		
		@return An error estimate.
		@throw std::runtime_error If could not converge.
		*/
		template <typename F, typename V, typename Norm>
		double integrate_with_error(F function, double x0, double x1, V& integral, Norm normalizator = std::abs) const;
	private:
		template <typename F, typename V, typename Norm>
		double integrate_impl(F function, double x0, double x1, unsigned int level, double current_tolerance, V& result, Norm normalizator) const;
		unsigned int max_subdiv_;
		double tolerance_;
		bool blow_up_;
		static const double start_;
		static const double end_;
		static const double xgk_[2*N+1];
		static const double wgk_[2*N+1];
		static const double xg_[N];
		static const double wg_[N];
	};

	template <size_t N>
	Adapt1D<N>::Adapt1D(unsigned int max_subdiv, double tolerance, bool blow_up)
	{
		max_subdiv_ = max_subdiv;
		tolerance_ = tolerance;
		blow_up_ = blow_up;
	}

	template <size_t N>
	template <typename F, typename V, typename Norm>
	void Adapt1D<N>::integrate(F function, double x0, double x1, V& result, Norm normalizator) const
	{
		const double error = integrate_with_error(function, x0, x1, result, normalizator);
		if (blow_up_ && error > tolerance_) {
			throw std::runtime_error("Could not converge: limit of subdivisions exceeded");
		}
	}

	template <size_t N>
	template <typename F>
	inline double Adapt1D<N>::integrate(F function, double x0, double x1) const
	{
		double result = 0.0;
		integrate<F,double,double(*)(double)>(function, x0, x1, result, std::abs);
		return result;
	}

	template <size_t N>
	template <typename F, typename V, typename Norm>
	double Adapt1D<N>::integrate_with_error(F function, double x0, double x1, V& integral, Norm normalizator) const
	{
		const double error = integrate_impl<F,V,Norm>(function, x0, x1, 0, tolerance_, integral, normalizator);
		return error;
	}

	template <size_t N>
	template <typename F, typename V, typename Norm>
	double Adapt1D<N>::integrate_impl(F function, double x0, double x1, unsigned int level, double current_tolerance, V& result, Norm normalizator) const
	{
		const double a = (x1 - x0) / (end_ - start_);
		assert( a > 0 );
		const double b = x0 - a * start_;
		result = function(b + a*xgk_[0]) * wgk_[0];
		V less_precise(function(b + a*xg_[0]) * wg_[0]);
		for (size_t i = 1; i < N; ++i) {
			less_precise += function(b + a*xg_[i]) * wg_[i];
			result += function(b + a*xgk_[i]) * wgk_[i];
		}
		for (size_t i = N; i <= 2*N; ++i) {
			result += function(b + a*xgk_[i]) * wgk_[i];
		}
		less_precise *= a;
		result *= a;
		const double error = normalizator(result - less_precise);
		if (error <= current_tolerance)
		{
			return error;
		} 
		else 
		{
			if (level < max_subdiv_)
			{
				const double mid_x = 0.5*x0 + 0.5*x1;
				if (mid_x > x0 && mid_x < x1) 
				{
					V left_integral;
					const double left_error = integrate_impl<F,V,Norm>(function, x0, mid_x, level + 1, current_tolerance / 2, left_integral, normalizator);
					V right_integral;
					const double right_error = integrate_impl<F,V,Norm>(function, mid_x, x1, level + 1, current_tolerance / 2, right_integral, normalizator);
					result = left_integral + right_integral;
					return left_error + right_error;
				} 
				else 
				{
					return error;
				}
			}
			else 
			{
				return error;
			}
		}
	}

	typedef Adapt1D<7> Adapt1D_15;
	typedef Adapt1D<20> Adapt1D_41;

}

#endif // __AVERISERA_ADAPT_1D_H
