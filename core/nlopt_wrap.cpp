#include "log.hpp"
#include "nlopt_wrap.hpp"
#include <boost/format.hpp>


namespace nlopt {
	const char* retcodestr(result ret) {
		switch (ret) {
		case FAILURE:
			return "FAILURE";
		case INVALID_ARGS:
			return "INVALID";
		case OUT_OF_MEMORY:
			return "OUT_OF_MEMORY";
		case ROUNDOFF_LIMITED:
			return "ROUNDOFF_LIMITED";
		case FORCED_STOP:
			return "FORCED_STOP";
		case SUCCESS:
			return "SUCCESS";
		case STOPVAL_REACHED:
			return "STOPVAL_REACHED";
		case FTOL_REACHED:
			return "FTOL_REACHED";
		case XTOL_REACHED:
			return "XTOL_REACHED";
		case MAXEVAL_REACHED:
			return "MAXEVAL_REACHED";
		case MAXTIME_REACHED:
			return "MAXTIME_REACHED";
		default:
			return "UNKNOWN";
		}
	}
}

namespace averisera {

	static void log_roundoff_limited(const std::string& problem_name, double value) {
		LOG_WARN() << problem_name << " roundoff limited with result: " << value;
	}

	nlopt::result run_nlopt(const std::string& problem_name, nlopt::opt& opt, std::vector<double>& x, double& value) {
		try {
			const nlopt::result result = opt.optimize(x, value);
			const std::string result_str(nlopt::retcodestr(result));
			if (result < 0) {
				if (result != nlopt::ROUNDOFF_LIMITED) {					
					LOG_WARN() << problem_name << " optimization failed due to with status: " << result_str;
					throw std::runtime_error((boost::format("%s optimization failure: %s in algorithm %s") % problem_name % result_str % opt.get_algorithm_name()).str());
				} else {
					// roundoff_limited errors are USUALLY benign and still yield a good result
					log_roundoff_limited(problem_name, value);					
				}
			} else {
				LOG_INFO() << problem_name << " optimization successful with status: " << result_str;
			}
			return result;
		} catch (nlopt::roundoff_limited&) {
			// roundoff_limited errors are USUALLY benign and still yield a good result
			log_roundoff_limited(problem_name, value);
			return nlopt::ROUNDOFF_LIMITED;
		} catch (std::runtime_error& e) {
			LOG_ERROR() << problem_name << " optimizer error: " << e.what() << " in algorithm " << opt.get_algorithm_name();
			throw std::runtime_error((boost::format("%s optimizer error: %s in algorithm %s") % problem_name % e.what() % opt.get_algorithm_name()).str());
		}
	}

	StoppingConditions::StoppingConditions()
		: stopval(+HUGE_VAL), ftol_abs(0), ftol_rel(0), xtol_abs(0), xtol_rel(0), maxeval(0), maxtime(0) {}
}
