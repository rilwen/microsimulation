// (C) Averisera Ltd 2014-2020
#include "relative_risk_calibrator.hpp"
#include "core/data_exception.hpp"
#include <numeric>
#include <stdexcept>
#include <boost/format.hpp>

namespace averisera {
	namespace microsim {
		namespace RelativeRiskCalibrator {
			double calc_base_risk(const double avg_risk, const std::vector<double>& group_fractions, const std::vector<double>& group_relative_risks) {
				if (!(avg_risk >= 0 && avg_risk <= 1)) {
					throw std::domain_error("RelativeRiskCalibrator: average risk not in [0, 1]");
				}
				if (group_fractions.size() != group_relative_risks.size()) {
					throw std::domain_error(boost::str(boost::format("RelativeRiskCalibrator: got %d group fractions but %d group relative risks") % group_fractions.size() % group_relative_risks.size()));
				}
				const double sum_w = std::accumulate(group_fractions.begin(), group_fractions.end(), 0.0);
				if (sum_w > 1) {
					throw DataException(boost::str(boost::format("RelativeRiskCalibrator: sum of group factions above 1: %g") % sum_w));
				}
				const double sum_w_r = std::inner_product(group_fractions.begin(), group_fractions.end(), group_relative_risks.begin(), 0.0);
				if (sum_w_r < 0) {
					throw DataException(boost::str(boost::format("RelativeRiskCalibrator: inner product of group fractions and relative risks is negative: %g") % sum_w_r));
				}
				const double tmp = sum_w_r + (1 - sum_w);
				double base_risk;
				if (tmp != 0) {
					base_risk = avg_risk / tmp;
					if (!(base_risk >= 0 && base_risk <= 1)) {
						throw DataException(boost::str(boost::format("RelativeRiskCalibrator: data require base risk outside [0, 1]: %g") % base_risk));
					}
				} else {
					if (avg_risk == 0) {
						base_risk = 0;
					} else {
						throw DataException("RelativeRiskCalibrator: relative risks for all population are zero but non-zero average risk");
					}
				}
				return base_risk;
			}

			std::vector<double> calc_base_risks(const std::vector<double>& avg_risks, const Eigen::MatrixXd& group_fractions, const Eigen::MatrixXd& group_relative_risks) {
				const size_t nsets = avg_risks.size();
				if ((nsets != static_cast<size_t>(group_fractions.rows()) 
					|| (group_fractions.rows() != group_relative_risks.rows()))
					|| (group_fractions.cols() != group_relative_risks.cols()) ) {
					throw std::domain_error("RelativeRiskCalibrator: dimension mismatch");
				}
				std::vector<double> base_risks(nsets);
				const size_t ngrps = static_cast<size_t>(group_fractions.cols());
				std::vector<double> gf(ngrps);
				std::vector<double> grr(ngrps);
				for (size_t i = 0; i < nsets; ++i) {
					for (size_t j = 0; j < ngrps; ++j) {
						gf[j] = group_fractions(i, j);
						grr[j] = group_relative_risks(i, j);
					}
					base_risks[i] = calc_base_risk(avg_risks[i], gf, grr);
				}
				return base_risks;
			}
		}
	}
}
