#pragma once
#include <stdexcept>

namespace averisera {
	/** Exception thrown inside the program when we have a problem with the data which is not detectable easily/cheaply earlier 
	(thus the code cannot be expected to detect it earlier and thus it is not a logic_error).
	*/
	class DataException : public std::runtime_error {
	public:
		explicit DataException(const std::string& what)
			: std::runtime_error(what) {}
		explicit DataException(const char* what)
			: std::runtime_error(what) {}
	};
}
