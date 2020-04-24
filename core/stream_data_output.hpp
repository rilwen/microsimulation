/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_STREAM_DATA_OUTPUT_H
#define __AVERISERA_STREAM_DATA_OUTPUT_H

#include "data_output.hpp"
#include <ostream>

namespace averisera {
	// Data output mechanism which writes them to a stream.
	class StreamDataOutput: public DataOutput {
		public:
			// stream - pointer to an output stream
			// delimiter - character separating fields
			// precision - precision of the numerical output
			StreamDataOutput(std::ostream* stream, char delimiter = '\t', int precision = 5);
			void output_column_names(const std::vector<std::string>& names);
			void output_data_row(const std::vector<double>& row);
		private:
			std::ostream* _stream;
			char _delimiter;
			int _precision;
	};
}

#endif