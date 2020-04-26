// (C) Averisera Ltd 2014-2020
#include "log.hpp"
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/format.hpp>
#include <stdexcept>

namespace averisera {
	namespace Logging {
		void set_level(const std::string& level) {
			if (level == "TRACE") {
				boost::log::core::get()->set_filter(
					boost::log::trivial::severity >= boost::log::trivial::trace
					);
			} else if (level == "DEBUG") {
				boost::log::core::get()->set_filter(
					boost::log::trivial::severity >= boost::log::trivial::debug
					);
			} else if (level == "INFO") {
				boost::log::core::get()->set_filter(
					boost::log::trivial::severity >= boost::log::trivial::info
					);
			} else if (level == "WARN") {
				boost::log::core::get()->set_filter(
					boost::log::trivial::severity >= boost::log::trivial::warning
					);
			} else if (level == "ERROR") {
				boost::log::core::get()->set_filter(
					boost::log::trivial::severity >= boost::log::trivial::error
					);
			} else if (level == "FATAL") {
				boost::log::core::get()->set_filter(
					boost::log::trivial::severity >= boost::log::trivial::fatal
					);
			} else {
				throw std::domain_error(boost::str(boost::format("Log: unknown level %s (supported levels TRACE, DEBUG, INFO, WARN, ERROR and FATAL)") % level));
			}
		}
	}
}
