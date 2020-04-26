// (C) Averisera Ltd 2014-2020
#ifndef __AVERISERA_CSM_PARAMS_H
#define __AVERISERA_CSM_PARAMS_H
#include <memory>

namespace averisera {
	class CSMRegulariser;
    struct ObservedDiscreteData;

    /** \class Hyperparameters of a CSM model. Does not include calibration stopping conditions. */
    struct CSMParams {  
        unsigned int memory; /**< Memory length */
        
        double tr_prob_nn; /**< Bound on probabilities of transition between non-neighbouring states. */

        unsigned int dim; /**< Dimension of the process to be assumed. If 0, CSM calculates it from the data. */

        double regularisation_lambda; /**< Regularisation strength. */

		std::shared_ptr<const CSMRegulariser> regulariser; /// Regulariser implementation.

        /**
		@param n_memory Memory length.
		@param n_tr_prob_nn Bound on probabilities of transition between non-neighbouring states.
		@param n_dim Dimension of the process to be assumed (pass 0 to calculate from data).
		@param n_regularisation_lambda Regularisation strength, >= 0.
		@param n_regulariser Regulariser implementation. Can be null only for n_regularisation_lambda == 0.
		@throw std::invalid_argument See #validate().
        @throw std::out_of_range See #validate().
        */
        CSMParams(unsigned int n_memory, double n_tr_prob_nn, unsigned int n_dim, double n_regularisation_lambda, std::shared_ptr<const CSMRegulariser> n_regulariser);

        CSMParams(const CSMParams& other) = default;

        /** Return a copy of parameters with dim set from data but only if it was originally 0.
         */
        CSMParams with_dim(const ObservedDiscreteData& data) const;

		/** Validate that parameters are correct. 
		@throw std::invalid_argument If tr_prob_nn > 0 for memory > 0, or regularisation_lambda != 0 for regulariser == nullptr.
		@throw std::out_of_range If tr_prob_nn outside [0, 1] or regularisation_lambda < 0.
		*/
		void validate() const;
    };
}

#endif // __AVERISERA_CSM_PARAMS_H
