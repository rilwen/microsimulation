#include "csm_params.hpp"
#include <stdexcept>
#include "observed_discrete_data.hpp"

namespace averisera {
    CSMParams::CSMParams(unsigned int n_memory, double n_tr_prob_nn, unsigned int n_dim,
		double n_regularisation_lambda, std::shared_ptr<const CSMRegulariser> n_regulariser)
        : memory(n_memory),
		tr_prob_nn(n_tr_prob_nn),
		dim(n_dim),
		regularisation_lambda(n_regularisation_lambda),
		regulariser(n_regulariser) {        
		validate();
    }

    CSMParams CSMParams::with_dim(const ObservedDiscreteData& data) const {
        CSMParams params(*this);
        if (params.dim == 0) {
            params.dim = static_cast<unsigned int>(ObservedDiscreteData::dim(data));
        }
        return params;
    }

	void CSMParams::validate() const {
		if (tr_prob_nn < 0.0 || tr_prob_nn > 1.0) {
			throw std::out_of_range("CSMParams: probability bound out of range");
		}
		if (regularisation_lambda < 0) {
			throw std::out_of_range("CSMParams: regularisation lambda is negative");
		}
		if (regularisation_lambda > 0 && !regulariser) {
			throw std::invalid_argument("CSMParams: regularisation strenght non-zero but regulariser is null");
		}
	}
}
