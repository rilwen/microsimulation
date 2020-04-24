#ifndef __AVERISERA_ADAPT_H
#define __AVERISERA_ADAPT_H

#include <cassert>
#include <cstring>
#include <vector>
#include "adapt_dbl.h"

namespace averisera {
    /** Multidimensional adaptive integration over a hypercube using the module ADAPT from CMLIB */
    class AdaptIntegration {
    public:
        static const unsigned int DEFAULT_MAXPTS_TO_RULCLS = 100;

        typedef double (*integrand_t)(const int*, const double*, const int*, const double*);
        
        /** @param dim Dimension of the integral, > 1
          @param eps Required relative accuracy
          @param maxpts_to_rulcls How many more maximum function evaluations over "theoretical" number
          Assumes integration over [0,1]^dim hypercube

          @throw std::domain_error If dim <= 1, eps <= 0 or maxpts_to_rulcls < 1
        */
        AdaptIntegration(unsigned int dim, double eps, unsigned int maxpts_to_rulcls = DEFAULT_MAXPTS_TO_RULCLS);

        /** @param dim Dimension of the integral
          @param eps Required relative accuracy
          @param a Vector of lower bounds of size dim
          @param b Vector of upper bounds of size dim
          @param maxpts_to_rulcls How many more maximum function evaluations over "theoretical" number

          @throw std::domain_error If dim < 1, eps <= 0, a.size() != dim, b.size() != dim or any a[i] >= b[i]
          or maxpts_to_rulcls < 1.
        */
        AdaptIntegration(unsigned int dim, double eps, const std::vector<double>& a, const std::vector<double>& b,
                         unsigned int maxpts_to_rulcls = DEFAULT_MAXPTS_TO_RULCLS);

        void reset_minpts() {
            _minpts = 1;
        }

        unsigned int minpts() const {
            assert(_minpts >= 0);
            return static_cast<unsigned int>(_minpts);
        }

        /** Integrate a function 
          @param integrand Integrated function with parameters (dim, x, nparam, params)
          Which takes argument x of size dim and parameter array params of size nparam
          @param[in] integrand Integrated function
          @param[in] params Vector of double parameters
          @param[out] relerr Estimated relative error of the integral
          @param[out] ifail Error code: 0 for success, 1 when not enough function evaluatins
          available to obtain desired accuracy
          @throw std::logic_error when internal settings are incorrect
         */
        double integrate(integrand_t integrand, const std::vector<double>& params, double& relerr, int& ifail);

        /** Integrate a function which takes a generic parameter passed via a double array. Example:

          struct data {
              double a;
              double b;
          };
          
          static double function_uses_struct(const unsigned int* dim, const double* x, const unsigned int* npara, const double* params) {
              assert(*npara == 1);
              const void * p = AdaptIntegration::get_ptr(params[0]);
              const data* d = static_cast<const data*>(p);
              return d->a + d->b;
          }

          @param[in] integrand Integrated function
          @param[in] param Pointer to param (can be null but integrand has to be able to handle that)
          @param[out] relerr Estimated relative error of the integral
          @param[out] ifail Error code: 0 for success, 1 when not enough function evaluatins
          available to obtain desired accuracy
          @throw std::logic_error when internal settings are incorrect
         */
        double integrate(integrand_t integrand, const void* params, double& relerr, int& ifail);

        /** Helper function to get const void* pointer from a double variable inside integrated function.         
         */
        static const void* get_ptr(const double& d);
    private:
        /** Helper function to place a const void* pointer inside a double variable as a para for the integrated function.         
         */
        static double set_ptr(const void* ptr);
        void handle_result(int ifail, double relerr, double result) const;
        //void poison_work_vectors();
        //void initialize_work_vectors(double val);

        integer _dim;
        double _eps;
        std::vector<double> _a;
        std::vector<double> _b;
		integer _minpts;
		integer _maxpts;
		integer _lenwrk;
        std::vector<double> _wrkstr;
    };
}

#endif // __AVERISERA_ADAPT_H
