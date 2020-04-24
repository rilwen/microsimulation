#ifndef __AVERISERA_DISTRIBUTION_EMPIRICAL_H
#define __AVERISERA_DISTRIBUTION_EMPIRICAL_H

#include "distribution.hpp"
#include <vector>

namespace averisera {
    /** Empirical distribution constructed from a provided sample */
    class DistributionEmpirical: public Distribution {
    public:
        /** @throw std::domain_error When values.empty()
         */
        DistributionEmpirical(const std::vector<double>& values);

        /** @throw std::domain_error When values.empty()
         */        
        DistributionEmpirical(std::vector<double>&& values);

		/** Construct from objects carrying values 
		@throw std::domain_error When objects.empty()
		*/
		template <class T, class G> DistributionEmpirical(const std::vector<T>& objects, G value_getter) {
			_values.reserve(objects.size());
			for (const T& t : objects) {
				_values.push_back(value_getter(t));
			}
			validate(_values);
			process();
		}

        /** @throw std::domain_error When begin == end
         */
        template <class I> DistributionEmpirical(I begin, I end);

        double pdf(double x) const override;

        double cdf(double x) const override;

		double cdf2(double x) const override;

        double icdf(double p) const override;

        double draw(RNG& rng) const override;

        double mean() const override {
            return _mean;
        }

        double variance(double mean) const override {
            return _variance;
        }

		double infimum() const override {
			return _values.front();
		}

		double supremum() const override {
			return _values.back();
		}
    private:
        static void validate(const std::vector<double>& values);
        void process();
        
        std::vector<double> _values;
        size_t _sample_size;
        double _mean;
        double _variance;
    };

    template <class I> DistributionEmpirical::DistributionEmpirical(I begin, I end)
        : _values(begin, end) {
        validate(_values);
        process();
    }
}

#endif // __AVERISERA_DISTRIBUTION_EMPIRICAL_H
