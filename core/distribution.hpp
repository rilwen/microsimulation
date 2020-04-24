/*
  (C) Averisera Ltd 2014
  Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_DISTRIBUTION_H
#define __AVERISERA_DISTRIBUTION_H

#include "multivariate_distribution.hpp"
#include "sampling_distribution.hpp"
#include "rng.hpp"
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <vector>

namespace averisera {
    /** @brief Represents a real-valued probability distribution */
    class Distribution: public MultivariateDistribution, public SamplingDistribution {
    public:
        // Calculate probability density function at x
        // x: Value of random variable
		// @return: For discrete distributions, return +infty if P(x) != 0, 0 otherwise
        virtual double pdf(double x) const = 0;

        /** Calculate cumulative distribution function at x
        @param x Upper value of random variable
        @return P(X <= x)
        */
        virtual double cdf(double x) const = 0;

        /** Calculate P(X < x)
          (Note the sharp inequality)
          Default implementation equal to cdf(x) := P(X <= x)
        */
        virtual double cdf2(double x) const {
            return cdf(x);
        }

        /** @brief Calculate P(x1 < X <= x2)

          Default implementation is range_prob(x1, x2) = cdf(x2) - cdf(x1) if x2 >= x1 or 0 if x2 < x1.

          @return 0 if x2 < x1
        */
        virtual double range_prob(double x1, double x2) const;

        /** @brief Calculate P(x1 <= X < x2)

          Default implementation is range_prob2(x1, x2) = cdf2(x2) - cdf2(x1) if x2 >= x1 or 0 if x2 < x1.

          @return 0 if x2 < x1
        */
        virtual double range_prob2(double x1, double x2) const;

        /** Calculate the inverse of cumulative distribution function.
          @param[in] p Probability
          @throw std::out_of_range If p < 0 or p > 1
          @return such x that P(X <= x) == p. Never return a lower number than the infimum of the distribution's range
          or a higher than its supremum. For discrete distribution P(X = x_k) = p_k, k = 1, ..., N, icfd(0 <= p <= p_1) == x_1 and icdf(1 - p_N < p <= 1) == x_N.
        */
        virtual double icdf(double p) const = 0;

        /** Default implementation uses inverse CDF and U(0,1) random numbers. */
        double draw(RNG& rng) const override {
            return icdf(rng.next_uniform());
        }

        // Calculate cumulative probabilities for a discrete distribution.
        // p: Vector of probabilities for each category. Cannot be empty.
        // cp: Vector of the same size as p. At exit, cp[i] == sum_{j=0}^i p[j], and cp[p.size() - 1] == 1.0
		// tol: Tolerance for sum of probabilities. If non-negative, throw an exception if abs(sum(p) - 1) > tol
        static void calculate_cumulative_proba(const std::vector<double>& p, std::vector<double>& cp, double tol = -1);

        size_t dim() const override {
            return 1;
        }

        void draw(RNG& rng, Eigen::Ref<Eigen::VectorXd> x) const override;

        void draw_noncont(RNG& rng, Eigen::Ref<Eigen::VectorXd, 0, Eigen::InnerStride<>> x) const override;

        void marginal_cdf(Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> p) const override;

        void marginal_icdf(Eigen::Ref<const Eigen::VectorXd> p, Eigen::Ref<Eigen::VectorXd> x) const override;

        void adjust_distribution(Eigen::Ref<Eigen::MatrixXd> sample) const override;

        /** Calculate the mean. Default implementation does a numerical integral each time it is called. 

          Distributions without a mean (e.g. Levy) should return NaN, or a pseudo-mean (e.g. a symmetric Levy).
         */
        virtual double mean() const;

        /** Calculate the variance given the mean. Default implementation does a numerical integral each time it is called. 

          Distributions without a variance (e.g. Cauchy) should return Infinity. Should never return a negative value.
         */
        virtual double variance(double mean) const;

        /** E[X | X \in [a, b)] 
          Works only for continuous CDFs
          @throw std::domain_error If b <= a
         */
        virtual double conditional_mean(double a, double b) const;

        /** Var[X | X \in [a, b)] 
          Works only for continuous CDFs
          @param conditional_mean E[X | X \in [a, b)]  
          @throw std::domain_error If b <= a or conditional_mean outside [a, b)
         */
        virtual double conditional_variance(double conditional_mean, double a, double b) const;

        ///** Create a heap-allocated exact copy. Never returns null. */
        //virtual std::unique_ptr<Distribution> clone() const = 0;

        /** Return idx such that xt[idx - 1] < x <= xt[idx].
          If x <= xt.front(), returns 0.
          If x > xt.back(), returns xt.size().
         */
        static size_t idx(const std::vector<double>& xt, double x);

		/** Infimum of the range of values of the distribution */
		virtual double infimum() const = 0;

		/** Supremum of the range of values of the distribution */
		virtual double supremum() const = 0;

		/** Given the vector of range boundaries x and cdf[i] == P(X <= x[i]), interpolate the CDF on new range boundaries (linear interpolation).
		@param x Sorted vector with unique values
		@param cdf CDF vector, cdf[i] = P(X <= x[i]). cdf[0] == 0. cdf.back() == 1.0, cdf.size() == x.size()
		@param new_x Sorted vector with unique values containing all values of x and some intermediate values.
		@throw std::domain_error If above conditions are not met.
		*/
		static std::vector<double> interpolate_continuous_cdf(const std::vector<double>& x, const std::vector<double>& cdf, const std::vector<double>& new_x);

		/** Given vector of ranges x and two strictly increasing CDFs for distributions X1 and X2, derive a mapping X1 -> X2 by matching their CDFs. 
		@param x Sorted vector with unique values
		@param cdf1 Strictly increasing CDF vector, cdf1[i] = P(X <= x[i]). cdf1[0] == 0. cdf1.back() == 1.0, cdf1.size() == x.size()
		@param cdf2 Like cdf1
		@throw std::domain_error If above conditions are not met.
		*/
		static void map_values_via_cdfs(const std::vector<double>& x, const std::vector<double>& cdf1, const std::vector<double>& cdf2, std::vector<double>& x1, std::vector<double>& x2);		

		/** Create a function object which calculates distr->icdf 
		@param distr Distribution
		@throw std::domain_error If distr is null
		*/
		static std::function<double(double)> icdf_as_function(std::shared_ptr<const Distribution> distr);

		/** Create a function object which calculates distr->icdf
		@param distr Distribution
		@param upper_limit Cap the returned values at this value
		@throw std::domain_error If distr is null
		*/
		static std::function<double(double)> icdf_as_function(std::shared_ptr<const Distribution> distr, double upper_limit);
    protected:        
        void calc_prob_bounds(double& p0, double& p1) const {
            p0 = calc_prob_lower_bound();
            p1 = calc_prob_upper_bound();
        }

        void calc_prob_bounds(double& p0, double& p1, double a, double b) const;

        double calc_prob_lower_bound() const {
            return calc_prob_bound_impl(0, 1E-15);
        }

        double calc_prob_upper_bound() const {
            return calc_prob_bound_impl(1, 1 - 1E-15);
        }

        double calc_prob_bound_impl(double bnd_if_icdf_finite, double bnd_if_icdf_infinite) const;

        double calc_mean_integral(double p0, double p1) const;

        double calc_variance_integral(double p0, double p1, double mean) const;

		/**
		Validate vector of range boundaries and probabilities.
		*/
		static void validate(const std::vector<double>& x, const std::vector<double>& p);
    };
}

#endif
