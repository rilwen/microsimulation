// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_GENERIC_DISTRIBUTION_HPP
#define __AVERISERA_GENERIC_DISTRIBUTION_HPP

namespace averisera {
    class RNG;
    
    /** Univariate probability distribution for values of type T.
      @tparam T Value type with < and == relations ((integer numbers, real numbers, dates, etc.)
     */
    template <class T> class GenericDistribution {
    public:
        virtual ~GenericDistribution() {
        }

        /** Draw a random number from the distribution. */
        virtual T random(RNG& rng) const = 0;

		/** Create a conditional distribution for X in [left, right) 
        @throw std::runtime_error If P(X in [left, right)) == 0
        @return A naked pointer which ought to be dressed immediately.
        */
        virtual GenericDistribution<T>* conditional(T left, T right) const = 0;

        /** @brief Calculate P(x1 <= X < x2)
        @return 0 if x2 < x1
        */
        virtual double range_prob2(T x1, T x2) const = 0;

        /** Calculate the inverse of cumulative distribution function
        @param[in] p Probability
        @throw std::out_of_range If p < 0 or p > 1
        @return such x that P(X <= x) == p. Never return a lower number than the infimum of the distribution's range
          or a higher than its supremum. For discrete distribution P(X = x_k) = p_k, k = 1, ..., N, icfd(0 <= p <= p_1) == x_1 and icdf(1 - p_N < p <= 1) == x_N.
        */
        virtual T icdf_generic(double p) const = 0;

        /** Infimum of the range of values of the distribution */
        virtual T lower_bound() const = 0;

        /** Supremum of the range of values of the distribution */
        virtual T upper_bound() const = 0;
    };
}

#endif // __AVERISERA_GENERIC_DISTRIBUTION_HPP
