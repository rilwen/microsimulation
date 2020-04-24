#ifndef __AVERISERA_MS_ROOT_FINDING_H
#define __AVERISERA_MS_ROOT_FINDING_H

#include <cmath>
#include <stdexcept>

namespace averisera {
    namespace RootFinding {
        /** Find the root by bisection. Narrow the [a, b] interval until either |b -a| < xtol or min(|f(a)|, |f(b)|) < ytol
          @param[in] f Calculates f(x)
          @param[in] xtol Interval tolerance
          @param[in] ytol Value tolerance
          @param[in] maxeval Maximum number of function evaluations
          @param[in, out] a Lower bound
          @param[in, out] b Upper bound
          @param[out] eval Number of function evaluations performed
          @throw std::domain_error If a and b do not bracket a root or (xtol < 0 and ytol < 0).
          @return true if bracketed a root within tolerance, false if not.
        */
        template <class F> bool bisection(F f, const double xtol, const double ytol, const unsigned int maxeval, double& a, double& b, unsigned int& eval) {
            if (xtol < 0 && ytol < 0) {
                throw std::domain_error("bisection: xtol and ytol both negative");
            }
            double fa = f(a);
            double fb = f(b);
            if (fa * fb > 0) {
                throw std::domain_error("bisection: a and b do not bracket a root");
            }
            if (fabs(fa) <= ytol || fabs(fb) <= ytol) {
                return true;
            }
            eval = 2;
            while (eval < maxeval && fabs(b - a) > xtol) {
                const double c = a + (b - a) / 2;
                const double fc = f(c);
                ++eval;
                if (fa * fc <= 0) {
                    b = c;
                    fb = fc;
                } else {
                    // assume f(x) is continuous
                    a = c;
                    fa = fc;
                }
                if (fabs(fa) <= ytol || fabs(fb) <= ytol) {
                    return true;
                }
            }
            return fabs(b - a) <= xtol;
        }
    }
}

#endif // __AVERISERA_MS_ROOT_FINDING_H
