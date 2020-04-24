#pragma once
#include <boost/log/trivial.hpp>

namespace averisera {
#define LOG_TRACE() BOOST_LOG_TRIVIAL(trace)
#define LOG_DEBUG() BOOST_LOG_TRIVIAL(debug)
#define LOG_INFO() BOOST_LOG_TRIVIAL(info)
#define LOG_WARN() BOOST_LOG_TRIVIAL(warning)
#define LOG_ERROR() BOOST_LOG_TRIVIAL(error)
#define LOG_FATAL() BOOST_LOG_TRIVIAL(fatal)

	namespace Logging {
		/** Supported levels: TRACE, DEBUG, INFO, WARN, ERROR, FATAL 
		@throw std::domain_error If level not supported
		*/
		void set_level(const std::string& level);
	}
}
