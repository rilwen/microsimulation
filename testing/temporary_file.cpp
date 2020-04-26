// (C) Averisera Ltd 2014-2020
#include "temporary_file.hpp"
#include <cstdio>
#include <stdexcept>
#include <fstream>
#include <iostream>

namespace averisera {
	namespace testing {

		TemporaryFile::TemporaryFile() {
			char buffer[L_tmpnam];
			tmpnam(buffer); // get a safe filename for a temporary file
			filename = buffer;
			FILE* file = fopen(buffer, "w");
			if (file == 0) {
				throw std::runtime_error("Could not open temporary file");
			}
			fclose(file);
		}

		TemporaryFile::~TemporaryFile() {
			if (remove(filename.c_str())) {
				//throw std::runtime_error(std::string("Could not remove temporary file ") + filename);
				std::cerr << "Could not remove temporary file: " << filename << std::endl;
			}
		}

		TemporaryFileWithData::TemporaryFileWithData(const char* data) {
			std::ofstream outf(filename);
			outf << data;
		}

	}
}
