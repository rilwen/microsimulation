#ifndef __AVERISERA_MS_FUNCTOR_H
#define __AVERISERA_MS_FUNCTOR_H

#include <vector>
#include "feature_user.hpp"

namespace averisera {
    namespace microsim {
	class Contexts;
        class Feature;

	/*! \brief Context-sensitive function
	  \tparam A argument type
	  \tparam R result type
	 */
	template <class A, class R> class Functor: public FeatureUser<Feature> {
	public:
	    virtual ~Functor() {}

	    virtual R operator()(const A& arg, const Contexts& ctx) const = 0;
	};
    }
}

#endif // __AVERISERA_MS_FUNCTOR_H
