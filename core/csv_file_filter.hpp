#ifndef __AVERISERA_CSV_FILE_FILTER_H
#define __AVERISERA_CSV_FILE_FILTER_H

namespace averisera {
	/** Forward declarations used to minimize use of header files */
	class CSVFileReader;
	class DataOutput;
	
	/** Abstract filter for CSV files. */
	class CSVFileFilter {
		public:
			/** Apply the filter to the reader and direct the data to output
			* Resets the reader to read from the beginning
			* TODO: add the option to avoid resetting */
			virtual void apply(CSVFileReader& reader, DataOutput& output) = 0;
			
			/** Virtual destructor */
			virtual ~CSVFileFilter() {}
	};
}

#endif
