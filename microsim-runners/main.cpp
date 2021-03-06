/*
(C) Averisera Ltd 2014-2020
*/
#include "core/log.hpp"
#include "core/user_arguments.hpp"
#include <iostream>
#include <stdexcept>

/** Actual main function, responsible for interpreting user arguments. */
void do_main(const averisera::UserArguments& ua);

/** Main function template which sets up logging and trapping errors.

Reads user arguments from provided parameter file.
*/
int main(int argc, char* argv[]) {
	if (argc <= 1) {
		std::cerr << "Usage: " << argv[0] << " parameter_file" << std::endl;
		return -1;
	}
	averisera::UserArguments ua(argv[1]);
	const std::string log_level(ua.get("LOG_LEVEL", std::string("INFO")));
	averisera::Logging::set_level(log_level);
	const bool catch_exceptions = ua.get("CATCH_EXCEPTIONS", true);
	int errcode = 0;
	if (catch_exceptions) {
		try {
			do_main(ua);
		} catch (std::exception& e) {
			LOG_FATAL() << "Error: " << e.what();
			std::cerr << "Error: " << e.what() << std::endl;
			errcode = -2;
		}
	} else {
		do_main(ua);
	}
	return errcode;
}
