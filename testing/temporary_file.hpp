#ifndef __AVERISERA_TESTING_TEMPORARY_FILE_H
#define __AVERISERA_TESTING_TEMPORARY_FILE_H

#include <string>



namespace averisera {
	namespace testing {
		struct TemporaryFile {
		public:
			TemporaryFile();
			~TemporaryFile();

			std::string filename;
		};		

		struct TemporaryFileWithData: public TemporaryFile {
		public:
			TemporaryFileWithData(const char* data);
		};
	}
}

#endif