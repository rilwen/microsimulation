#include "f2c_types.hpp"

#ifdef __cplusplus
extern "C" {
#endif

    typedef doublereal (*adapt_fp)(const integer*, const doublereal*, const integer*, const doublereal*);
    
    int adapt_(integer *ndim, doublereal *a, doublereal *b, 
               integer *minpts, integer *maxpts, adapt_fp functn, doublereal *eps, 
               doublereal *relerr, integer *lenwrk, doublereal *wrkstr, doublereal *
               finest, integer *ifail, const doublereal *params, const integer *npara);
    
#ifdef __cplusplus
}
#endif
