/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "stream_data_output.hpp"

namespace averisera {
	StreamDataOutput::StreamDataOutput(std::ostream* stream, char delimiter, int precision)
	: _stream(stream), _delimiter(delimiter), _precision(precision)
	{
	}
	
	void StreamDataOutput::output_column_names(const std::vector<std::string>& names)
	{
		const size_t n = names.size();
		if (n > 0) {
			(*_stream) << names[0];
			for (size_t i = 1; i < n; ++i) {
				(*_stream) << _delimiter << names[i];
			}			
		}
		(*_stream) << std::endl;
	}
	
	void StreamDataOutput::output_data_row(const std::vector<double>& row)
	{
		const size_t n = row.size();
		if (n > 0) {
			// do not output empty rows
			std::streamsize old_precision = _stream->precision();		
			_stream->precision(_precision);
			(*_stream) << row[0];
			for (size_t i = 1; i < n; ++i) {
				(*_stream) << _delimiter << row[i];
			}			
			(*_stream) << std::endl;
			_stream->precision(old_precision);			
		}		
	}
}
