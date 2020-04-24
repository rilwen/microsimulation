#ifndef __AVERISERA_DATA_OUTPUT
#define __AVERISERA_DATA_OUTPUT

#include <string>
#include <vector>

namespace averisera {
	/** Abstract sink for data */
	class DataOutput {
		public:
			/** Output column names */
			virtual void output_column_names(const std::vector<std::string>& names) = 0;
			
			/** Output data row */
			virtual void output_data_row(const std::vector<double>& row) = 0;
			
			/** Virtual destructor */
			virtual ~DataOutput() {}
	};
}

#endif
