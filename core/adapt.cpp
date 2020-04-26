/*
(C) Averisera Ltd 2014-2020
*/
#include "adapt.hpp"
#include "log.hpp"
#include "math_utils.hpp"
#include "rng.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>

// extern "C" {
//     void adapt_(const unsigned int* dim, const double* a, const double* b, unsigned int* minpts, const unsigned int* maxpts,
//                 double (*functn)(const unsigned int*, const double*, const unsigned int*, const double*), const double* eps, double* relerr,
//                 const unsigned int* lenwrk, double* wrkstr, double* finest, int* ifail, const double* params, const int* npara);
// }

namespace averisera {
    AdaptIntegration::AdaptIntegration(unsigned int dim, double eps, unsigned int maxpts_to_rulcls)
        : AdaptIntegration(dim, eps, std::vector<double>(dim, 0.0), std::vector<double>(dim, 1.0), maxpts_to_rulcls)
    {}

    static unsigned int calc_rulcls(unsigned int dim) {
        return static_cast<unsigned int>(pow(2, dim) + 2 * pow(dim, 2) + 6 * dim + 1);
    }

    static unsigned int calc_lenwrk(unsigned int dim, unsigned int maxpts_to_rulcls) {
        return (2 * dim + 3) * (1 + maxpts_to_rulcls) / 2;
    }

    AdaptIntegration::AdaptIntegration(const unsigned int dim, double eps, const std::vector<double>& a,
                                       const std::vector<double>& b, unsigned int maxpts_to_rulcls)
        : _dim(dim), _eps(eps), _a(a), _b(b), _minpts(1), _maxpts(maxpts_to_rulcls * calc_rulcls(dim)),
          _lenwrk(MathUtils::safe_cast<integer>(calc_lenwrk(dim, maxpts_to_rulcls))),
          _wrkstr(static_cast<size_t>(_lenwrk)) {
        if (dim <= 1) {
            throw std::domain_error("AdaptIntegration: dimension must be at least 2");
        }
        if (eps <= 0) {
            throw std::domain_error("AdaptIntegration: epsilon not positive");
        }
        if (MathUtils::safe_cast<unsigned int>(a.size()) != dim || MathUtils::safe_cast<unsigned int>(b.size()) != dim) {
            throw std::domain_error("AdaptIntegration: bad bounds vector size(s)");
        }
        for (unsigned int i = 0; i < dim; ++i) {
            if (a[i] >= b[i]) {
                throw std::domain_error("AdaptIntegration: bad bounds ordering");
            }
        }        
    }

    double AdaptIntegration::integrate(integrand_t integrand, const std::vector<double>& params, double& relerr, int& ifail) {
        double result;
        const integer npara = static_cast<integer>(params.size());
		integer _ifail;
        //poison_work_vectors();
        adapt_(&_dim, &_a[0], &_b[0], &_minpts, &_maxpts, (adapt_fp)integrand, &_eps, &relerr, &_lenwrk, &_wrkstr[0],
               &result, &_ifail, npara ? &params[0] : nullptr, &npara);
		ifail = _ifail;
        handle_result(ifail, relerr, result);
        return result;
    }

    double AdaptIntegration::integrate(integrand_t integrand, const void* params, double& relerr, int& ifail) {
        static_assert(sizeof(double) >= sizeof(void*), "void* pointer must fit in double value");
        double result;
		integer _ifail;
        static const integer one = 1;
        const double dummy_data = set_ptr(params);
        //poison_work_vectors();
        adapt_(&_dim, &_a[0], &_b[0], &_minpts, &_maxpts, (adapt_fp)integrand, &_eps, &relerr, &_lenwrk, &_wrkstr[0],
               &result, &_ifail, &dummy_data, &one);
		ifail = _ifail;
        handle_result(ifail, relerr, result);
        return result;
    }

    // static inline void fill(std::vector<double>& vec, double val) {
    //     std::fill(vec.begin(), vec.end(), val);
    // }

    // void AdaptIntegration::initialize_work_vectors(double val) {
    //     fill(_wrkstr, val);
    // }

    // void AdaptIntegration::poison_work_vectors() {
    //     initialize_work_vectors(std::numeric_limits<double>::signaling_NaN());
    // }

    const void* AdaptIntegration::get_ptr(const double& d) {
        const void* ptr;
        std::memcpy(&ptr, &d, sizeof ptr);
        return ptr;
    }

    double AdaptIntegration::set_ptr(const void* ptr) {
        double d;
        std::memcpy(&d, &ptr, sizeof ptr);
        return d;
    }

    void AdaptIntegration::handle_result(int ifail, double relerr, double result) const {
        if (ifail == 3 || ifail == 2) {
            // we should have prevented that.
            throw std::logic_error("AdaptIntegration: internal settings error");
        } else if (ifail == 1) {
            LOG_WARN() << "AdaptIntegration: ifail==1 for _eps==" << _eps << ", relerr==" << relerr << " and result==" << result;
        }
    }
}

