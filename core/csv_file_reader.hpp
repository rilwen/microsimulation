/*
  (C) Averisera Ltd 2014
  Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_CSV_FILE_READER_H
#define __AVERISERA_CSV_FILE_READER_H

#include "abstract_csv_line_parser.hpp"
#include "csv.hpp"
#include "data_exception.hpp"
#include "filter_iterator.hpp"
#include "input_iterator_utils.hpp"
#include "position_keeper_in.hpp"
#include <cassert>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

namespace averisera {    

	// TODO: handle files with more lines than the range of index_type

	/** Delimiter Separated Value file reader.
     * Supports large files (does not read the whole file into memory).
     * Assumes each row in the format Value<delimiter>Value<delimiter>Value...\n
     * where delimiter can be a tab, a comma, semicolon, ...
     * Optionally, first row is assumed to hold column names. */
	class CSVFileReader {
    public:
        typedef size_t index_type;

		/** Create a CSV file reader linked to a file
		@param file_name Name of the file
		@param has_names First row is assumed to contain column names
		@param line_parser Pointer to CSV line parser
		@throw std::domain_error If line_parser is null
		@throw std::runtime_error If file cannot be read
		*/
		CSVFileReader(const std::string& file_name, bool has_names, std::unique_ptr<AbstractCSVLineParser>&& line_parser);
        
        /** Assumes values are separated by any of the characters in the string delimiters
         * For tab-separated values, use it like that:  CSVFileReader reader("file.tab", "\t") */
		CSVFileReader(const std::string& file_name, CSV::Delimiter delimiter = CSV::Delimiter::TAB, CSV::QuoteCharacter quote_character = CSV::QuoteCharacter::DOUBLE_QUOTE, bool has_names = true);

		/** Reset line parser to the default CSV parser with this settings */
		void reset_line_parser(CSV::Delimiter delimiter = CSV::Delimiter::TAB, CSV::QuoteCharacter quote_character = CSV::QuoteCharacter::DOUBLE_QUOTE);

		/** Select columns with given indices (don't return any others) 
		Repeated calling of this function selects from already selected set, so select_columns({0,4,5,6,7}) followed by select_columns({1,2}) is equivalent to select_columns({4, 5})*/
		void select_columns(const std::unordered_set<AbstractCSVLineParser::col_idx_t>& sel_cols);

		/** Select columns with given names or indices (don't return any others). Repeated calling of this function selects from already selected set. 
		Preserves the file position
		@param sel_col_names Names
		@param sel_col_indices Indices (merged with names)
		@throw std::logic_error If has_names() == false 
		*/
		void select_columns(const std::unordered_set<std::string>& sel_col_names, const std::unordered_set<AbstractCSVLineParser::col_idx_t>& sel_col_indices = std::unordered_set<AbstractCSVLineParser::col_idx_t>());
			
        /** Read the column names from the first row and point the reader at first data row.
         * If file is assumed to have no column names, return an empty vector. */
        const std::vector<std::string>& read_column_names();
			
        /** Do we have another row of data to read? */
        bool has_next_data_row() const;

		const std::string& file_name() const {
			return _file_name;
		}

		bool has_names() const {
			return _has_names;
		}
			
        /** Read a row into vector "row" and return number of elements read.
         * If row does not have enough space, it will be resized.
         * Entries which are not possible to convert into double are returned as NaNs. */
        index_type read_data_row(std::vector<double>& row);

		/** Read row elements with given indices into vector "row".
		* If row does not have enough space, it will be resized.
		Return true if the line in file was not empty.
		* Entries which are not possible to convert into double are returned as NaNs. 
		@param indices Index vector
		@param fill_with_nans If true, replace missing elements' values with NaNs.
		@throw std::runtime_error If fill_with_nans == false and any index is larger than number of columns in file.
		*/
		bool read_data_row(const std::vector<index_type>& indices, bool fill_with_nans, std::vector<double>& row);

        /** Read a row into vector "row" and return number of elements read.
         * If row does not have enough space, it will be resized.
         * Does not convert strings. */
        index_type read_data_row(std::vector<std::string>& row);
			
        /** Reset the reader, pointing it at the first data row. */
        void to_data();
			
        /** Is the reader ready for reading data rows. If it returns true, you can start using has_next_data_row() and read_data_row() to read data from file. */
        bool at_data() const;

        /** Count the number of data rows (without converting them to double) and rewind the reader to the beginning of the data. */
        index_type count_data_rows();

        /** Count the number of columns and rewind the reader to the beginning of the data. */
        index_type count_columns();

        typedef std::unordered_map<std::string, index_type> index_map_type;
        typedef std::unordered_map<std::string, std::string> value_map_type;

        /** Given a vector of names {n_0, n_1, ...} return an unordered map {n_i -> i}.
          @throw std::runtime_error If duplicate names occur */
        static index_map_type map_names_to_indices(const std::vector<std::string>& names);

		/** Read column names and return their map. Leaves reader in the same state as read_column_names() */
		index_map_type read_column_names_map() {
			return map_names_to_indices(read_column_names());
		}

		/** Convert column names to column indices 
		@throw std::domain_error If some name has no mapping
		*/
		std::vector<index_type> static convert_names_to_indices(const index_map_type& name_map, const std::vector<std::string>& names);

		/** Convert column names to column indices . Leaves reader in the same state as read_column_names_map() 
		@throw std::domain_error If some name has no mapping
		*/
		std::vector<index_type> convert_names_to_indices(const std::vector<std::string>& names) {
			return convert_names_to_indices(read_column_names_map(), names);
		}

        /** Read values of named columns.
          @param[in] headers Maps column name -> column index
          @param[out] values Maps column name -> column value for every value read. If values contains a key which is not a key in headers, its value will not be changed.
          @return number of values read */
        index_type read_data_row(const index_map_type& headers, value_map_type& values);

		/** Check that given name is present in the map and throw std::runtime_error if it is not.
		@param caller Name of the calling class or method
		*/
		static void check_column_present(const std::string& caller, const index_map_type& map, const std::string& name, const std::string& filename);

		/** Check that the value for given column is present in the map and throw std::runtime_error if it is not.
		@param caller Name of the calling class or method
		@param allow_empty Are empty strings allowed as values
		*/
		static void check_value_present(const std::string& caller, const value_map_type& map, const std::string& name, const std::string& filename, bool allow_empty);

		template <class T, class Handler> static T convert_element(const std::string& elem, Handler handler) {
			try {
				return boost::lexical_cast<T>(elem);
			} catch (std::exception& e) {
				return handler(e, elem);
			}
		}

		/** InputIterator over file lines.
		Points either to the current line in file, or past the end of the file.
		*/
		class LineIterator: public std::iterator<std::input_iterator_tag, const std::vector<std::string>>  {
		public:
			friend class CSVFileReader;
			LineIterator(LineIterator&& other);
			LineIterator(const LineIterator& other);
			LineIterator& operator=(const LineIterator& other) = default;
			const std::vector<std::string>& operator*() const {
				assert(!_is_end);
				return _reader->_elements;
			}
			const std::vector<std::string>* operator->() const {
				assert(!_is_end);
				return &_reader->_elements;
			}
			friend LineIterator& operator++(LineIterator& iter);
			PostIncrementProxy<value_type> operator++(int);
			bool operator==(const LineIterator& other) const;
			bool operator!=(const LineIterator& other) const {
				return !((*this) == other);
			}
			bool is_end() const {
				return _is_end;
			}
			friend class StringIterator;
		private:
			/** Moving Iterator */
			LineIterator(CSVFileReader& reader);
			/** EOF iterator */
			LineIterator();
			void read_elements();
			CSVFileReader* _reader;
			bool _is_end; /**< True if iterator points past the end of the file or _reader is null. */
		};

		friend class LineIterator;

		LineIterator begin();

		LineIterator end() const {
			return LineIterator();
		}

        /** InputIterator over n-th string element of each file line. */
		class StringIterator: public std::iterator<std::input_iterator_tag, const std::string> {
		public:
			friend class CSVFileReader;
			StringIterator(StringIterator&& other);
			StringIterator(const StringIterator& other);
			StringIterator& operator=(const StringIterator& other) = default;
			value_type& operator*() const;
			value_type* operator->() const;
			friend StringIterator& operator++(StringIterator& iter);
			PostIncrementProxy<value_type> operator++(int);
			bool operator==(const StringIterator& other) const;
			bool operator!=(const StringIterator& other) const {
				return !((*this) == other);
			}
			bool is_end() const {
				return _line_iterator.is_end();
			}
		private:
			StringIterator(LineIterator&& line_iterator, index_type elem_idx);
			LineIterator _line_iterator;
			index_type _elem_idx;			
		};

		StringIterator begin(index_type elem_idx) {
			return StringIterator(begin(), elem_idx);
		}

		StringIterator end(index_type elem_idx) const {
			return StringIterator(end(), elem_idx);
		}

		/** InputIterator over n-th string element of each file line, converted to T. 
		@tparam C Converter Functor called to convert the read string to type T. If conversion fails, may return a default value or
		throw an exception.
		*/
		template <class T, class C> class ConvertingIterator: public std::iterator<std::input_iterator_tag, const T>{
		public:
			friend class CSVFileReader;
			ConvertingIterator(ConvertingIterator<T, C>&& other);
			ConvertingIterator(const ConvertingIterator<T, C>& other);
			ConvertingIterator<T, C>& operator=(const ConvertingIterator<T, C>& other) = default;
			typename ConvertingIterator<T, C>::value_type operator*() const;			
			ConvertingIterator<T, C>& operator++();
			PostIncrementProxy<typename ConvertingIterator<T, C>::value_type> operator++(int);
			bool operator==(const ConvertingIterator<T, C>& other) const;
			bool operator!=(const ConvertingIterator<T, C>& other) const {
				return !((*this) == other);
			}
			bool is_end() const {
				return _string_iterator.is_end();
			}
		private:
			StringIterator _string_iterator;
			C converter_;
		protected:
			ConvertingIterator<T, C>(StringIterator&& string_iterator, C converter);
		};

		template <class T, class C> ConvertingIterator<T, C> begin_converting(index_type elem_idx, C converter) {
			return ConvertingIterator<T, C>(begin(elem_idx), converter);
		}

		template <class T, class C> ConvertingIterator<T, C> end_converting(index_type elem_idx, C converter) const {
			return ConvertingIterator<T, C>(end(elem_idx), converter);			
		}

		template <class T, class Handler> class DefaultConverter {
		public:
			DefaultConverter(Handler handler)
				: handler_(handler) {}
			T operator()(const std::string& str) const {
				return convert_element<T, Handler>(str, handler_);
			}
		private:
			Handler handler_;
		};

		template <class T> class ReturnDefaultValueHandler {
		public:
			ReturnDefaultValueHandler(T dflt_val)
				: _dflt(dflt_val) {}
			T operator()(const std::exception&, const std::string&) const {
				return _dflt;
			}
		private:
			T _dflt;
		};

		struct ReturnNaNHandler {
			double operator()(const std::exception&, const std::string&) const {
				return std::numeric_limits<double>::quiet_NaN();
			}
		};

		/** Complains about inability to convert */
		template <class T> class ComplainingHandler {
		public:
			ComplainingHandler(const CSVFileReader& reader, index_type col_idx)
				: reader_(reader), col_idx_(col_idx) {}
			T operator()(const std::exception&, const std::string& elem) const {
				throw DataException(boost::str(boost::format("CSVFileReader: cannot convert \"%s\" in file %s, column %d") % elem % reader_.file_name() % col_idx_));
				return 0;
			}
		private:
			const CSVFileReader& reader_;
			index_type col_idx_;
		};

		template <class T> static ReturnDefaultValueHandler<T> return_default_value_handler(T dflt) {
			return ReturnDefaultValueHandler<T>(dflt);
		}

		template <class T, class H> static DefaultConverter<T, H> default_converter(H handler) {
			return DefaultConverter<T, H>(handler);
		}

		template <class T> DefaultConverter<T, ComplainingHandler<T>> default_converter_complaining(index_type elem_idx) const {
			return default_converter<T>(ComplainingHandler<T>(*this, elem_idx));
		}
		
		template <class T, class H> ConvertingIterator<T, DefaultConverter<T,H>> begin_default_converting(index_type elem_idx, H handler) {
			return begin_converting<T>(elem_idx, DefaultConverter<T, H>(handler));
		}

		template <class T, class H> ConvertingIterator<T, DefaultConverter<T, H>> end_default_converting(index_type elem_idx, H handler) const {
			return end_converting<T>(elem_idx, DefaultConverter<T, H>(handler));
		}

		template <class T> ConvertingIterator<T, DefaultConverter<T, ComplainingHandler<T>>> begin_default_converting(index_type elem_idx) {
			return begin_converting<T>(elem_idx, default_converter<T>(elem_idx));
		}

		template <class T> ConvertingIterator<T, DefaultConverter<T, ComplainingHandler<T>>> end_default_converting(index_type elem_idx) const {
			return end_converting<T>(elem_idx, default_converter<T>(elem_idx));
		}

		/** InputIterator over n-th string element of each file line, converted to double. */
		typedef ConvertingIterator<double, DefaultConverter<double, ReturnNaNHandler>> DoubleIterator;

		DoubleIterator begin_double(index_type elem_idx) {
			return begin_default_converting<double>(elem_idx, ReturnNaNHandler());
		}

		DoubleIterator end_double(index_type elem_idx) const {
			return end_default_converting<double>(elem_idx, ReturnNaNHandler());
		}

		/** Input iterator which converts selected columns to doubles. Behaves as read_data_row() when reading and
		converting. */
		class MultiDoubleIterator : public std::iterator<std::input_iterator_tag, const std::vector<double>> {
		public:
			friend class CSVFileReader;
			MultiDoubleIterator(MultiDoubleIterator&& other);
			MultiDoubleIterator(const MultiDoubleIterator& other);
			value_type& operator*() const {
				return _values;
			}
			value_type* operator->() const {
				return &_values;
			}
			friend MultiDoubleIterator& operator++(MultiDoubleIterator& iter);
			PostIncrementProxy<value_type> operator++(int);
			bool operator==(const MultiDoubleIterator& other) const;
			bool operator!=(const MultiDoubleIterator& other) const {
				return !((*this) == other);
			}
			bool is_end() const {
				return _line_iterator.is_end();
			}
		private:
			LineIterator _line_iterator;
			std::vector<index_type> _indices;
			std::vector<double> _values;
			bool _fill_with_nans;

			MultiDoubleIterator(LineIterator&& line_iterator, const std::vector<index_type>& indices, bool fill_with_nans);
			void convert();
		};

		MultiDoubleIterator begin_double(const std::vector<index_type>& indices, bool fill_with_nans) {
			return MultiDoubleIterator(begin(), indices, fill_with_nans);
		}

		MultiDoubleIterator end_double(const std::vector<index_type>& indices, bool fill_with_nans) const {
			return MultiDoubleIterator(end(), indices, fill_with_nans);
		}

		template <class I> struct IsCSVFileReaderIteratorDereferencable {
			bool operator()(const I& iter) const {
				return !iter.is_end();
			}
		};		

		/** Iterator which selects only some rows */
		template <class I, class P> FilterIterator<I, P, IsCSVFileReaderIteratorDereferencable<DoubleIterator>> begin_double(const I& iter, P pred) {			
			return make_filter_iterator(iter, pred, make_deref_checker<I>());
		}
    private:
		std::string _file_name;
        bool _has_names;
        std::ifstream _file; /** streams we read from */
        std::unique_ptr<AbstractCSVLineParser> _line_parser; /**< Splits lines into vectors of strings */
        //char _delimiter; /** string of delimiters (e.g. "\t" or "\t:") */
        bool _at_data; /** is the reader ready for reading data rows */
        std::string _line; /** buffer for the data read from file */
        std::vector<std::string> _elements; /** buffer for the elements in the line */
		index_type next_line_idx_; /**< Index of the next line in file to be read, incl. optional column names */
			
        void to_beginning(); /** move to the beginning of the stream	*/
        void read_line(); /** read a line from file, remove the unnecessary end of line characters (important under Cygwin) */
        void read_elements(); /** read a line from file using read_line() and split it into _element if not empty; otherwise set _elements to empty vector */
		bool read_data_row(const std::vector<std::string>& elements, const std::vector<index_type>& indices, bool fill_with_nans, std::vector<double>& values) const;
		template <class I> IsCSVFileReaderIteratorDereferencable<I> make_deref_checker() {
			return IsCSVFileReaderIteratorDereferencable<I>();
		}		

		/** Preserves the state of the reader */
		class StateKeeper: public PositionKeeperIn {
		public:
			StateKeeper(CSVFileReader& reader);
			~StateKeeper();
		private:
			CSVFileReader& reader_;
			index_type next_line_idx_;
			bool at_data_;
		};
	};

	template <class T, class C> CSVFileReader::ConvertingIterator<T, C>::ConvertingIterator(ConvertingIterator<T, C>&& other)
		: _string_iterator(std::move(other._string_iterator)), converter_(std::move(other.converter_)) {}

	template <class T, class C> CSVFileReader::ConvertingIterator<T, C>::ConvertingIterator(const ConvertingIterator<T, C>& other)
		: _string_iterator(other._string_iterator), converter_(other.converter_) {}

	template <class T, class C> typename CSVFileReader::ConvertingIterator<T, C>::value_type CSVFileReader::ConvertingIterator<T, C>::operator*() const {
		return converter_(*_string_iterator);
	}

	template <class T, class C> CSVFileReader::ConvertingIterator<T, C>& CSVFileReader::ConvertingIterator<T, C>::operator++() {
		++_string_iterator;
		return *this;
	}

	template <class T, class C> PostIncrementProxy<typename CSVFileReader::ConvertingIterator<T, C>::value_type> CSVFileReader::ConvertingIterator<T, C>::operator++(int) {
		PostIncrementProxy<typename ConvertingIterator<T, C>::value_type> v(this->operator*());
		++(*this);
		return v;
	}

	template <class T, class C> bool CSVFileReader::ConvertingIterator<T, C>::operator==(const ConvertingIterator& other) const {
		return (_string_iterator == other._string_iterator);
	}

	template <class T, class C> CSVFileReader::ConvertingIterator<T, C>::ConvertingIterator(StringIterator&& string_iterator, C converter)
		: _string_iterator(std::move(string_iterator)), converter_(converter) {}
}

#endif
