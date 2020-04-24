/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "data_exception.hpp"
#include "csv_file_reader.hpp"
#include "csv_line_parser_selected_cols.hpp"
#include "log.hpp"
#include "preconditions.hpp"
#include "stl_utils.hpp"
#include <cassert>
#include <string>
#include <boost/format.hpp>
#include <stdexcept>
#include <sstream>
#include <iostream>

namespace averisera {
	CSVFileReader::CSVFileReader(const std::string& file_name, CSV::Delimiter delimiter, CSV::QuoteCharacter quote_character, bool has_names)
        :  CSVFileReader(file_name, has_names, CSV::make_line_parser(delimiter, quote_character)) {
	}

	CSVFileReader::CSVFileReader(const std::string& file_name, bool has_names, std::unique_ptr<AbstractCSVLineParser>&& line_parser)
		: _file_name(file_name), _has_names(has_names), _file(file_name.c_str()), _at_data(!has_names), next_line_idx_(0) {
		check_not_null(line_parser, "CSVFileReader: null parser");
		if (!_file.is_open()) {			
			throw std::runtime_error(boost::str(boost::format("CSVFileReader: cannot open file: %s") % file_name));
		}
		_line_parser = std::move(line_parser);
	}

	void CSVFileReader::select_columns(const std::unordered_set<AbstractCSVLineParser::col_idx_t>& sel_cols) {
		_line_parser = std::unique_ptr<AbstractCSVLineParser>(new CSVLineParserSelectedCols(sel_cols, std::move(_line_parser)));
	}

	void CSVFileReader::select_columns(const std::unordered_set<std::string>& sel_col_names, const std::unordered_set<AbstractCSVLineParser::col_idx_t>& sel_col_indices) {
		check_that<std::logic_error>(has_names(), "CSVFileReader: cannot select columns by name if file has no names");
		std::unordered_set<AbstractCSVLineParser::col_idx_t> idx_set(sel_col_indices);
		{ // scope for state keeper
			StateKeeper sk(*this);
			const std::vector<std::string>& names = read_column_names();			
			for (const std::string& name : sel_col_names) {
				const auto it = std::find(names.begin(), names.end(), name);
				if (it != names.end()) {
					idx_set.insert(std::distance(names.begin(), it));
				} else {
					LOG_WARN() << "Selecting columns from file " << _file_name << ": no such column name: " << name;
				}
			}
		}
		select_columns(idx_set);
	}

	void CSVFileReader::reset_line_parser(CSV::Delimiter delimiter, CSV::QuoteCharacter quote_character) {
		_line_parser = CSV::make_line_parser(delimiter, quote_character);
	}
			
	const std::vector<std::string>& CSVFileReader::read_column_names()
	{
		to_beginning();
		if (_has_names) {
			read_elements();
		} else {
			LOG_WARN() << "CSVFileReader::read_column_names: reader set up not to expect names";
			_elements.resize(0);
		}
		_at_data = true;
		return _elements;
	}
	
	bool CSVFileReader::has_next_data_row() const
	{
		if (_at_data) {
			return _file.good();
		} else {
			return false;
		}
	}	
	
	CSVFileReader::index_type CSVFileReader::read_data_row(std::vector<double>& row)
	{
		read_elements();
		if (!_elements.empty()) {
			const index_type nbr_read = _elements.size();
			row.resize(nbr_read);
			bool empty_columns = false;
			for (index_type i = 0; i < nbr_read; ++i) {
				row[i] = convert_element<double>(_elements[i], [this, i, &empty_columns](const std::exception& e, const std::string& elem) {
					if (elem.empty()) {
						empty_columns = true;
					} else {
						LOG_WARN() << "File " << _file_name << ": problem reading column " << i << " in line \"" << _elements << "\": cannot convert " << elem << " to value due to exception: " << e.what();
					}
					return std::numeric_limits<double>::quiet_NaN();
				});
			}
			if (empty_columns) {
				assert(next_line_idx_ > 0);
				LOG_WARN() << "File " << _file_name << ": some columns were empty in line #" << (next_line_idx_ - 1);
			}
			return nbr_read;
		} else {
			row.clear();
			return 0;
		}
	}

	bool CSVFileReader::read_data_row(const std::vector<index_type>& indices, bool fill_with_nans, std::vector<double>& row) {
		read_elements();
		return read_data_row(_elements, indices, fill_with_nans, row);
	}

	bool CSVFileReader::read_data_row(const std::vector<std::string>& elements, const std::vector<index_type>& indices, bool fill_with_nans, std::vector<double>& values) const {
		if (!elements.empty()) {
			const index_type nbr_read = elements.size();
			values.resize(indices.size());
			index_type dest_idx = 0;
			bool empty_columns = false;
			for (index_type src_idx : indices) {
				if (src_idx >= nbr_read) {
					if (fill_with_nans) {
						values[dest_idx] = std::numeric_limits<double>::quiet_NaN();
					} else {
						throw DataException(boost::str(boost::format("CSVFileReader: requested column %d (0-based) but only %d read") % src_idx % nbr_read));
					}
				} else {
					values[dest_idx] = convert_element<double>(elements[src_idx], [this, &elements, src_idx, &empty_columns](const std::exception& e, const std::string& elem) {
						if (elem.empty()) {
							empty_columns = true;
						} else {
							LOG_WARN() << "File " << _file_name << ": problem reading column " << src_idx << " in line \"" << elements << "\": cannot convert " << elem << " to value due to exception: " << e.what();
						}
						return std::numeric_limits<double>::quiet_NaN();
					});
				}
				++dest_idx;
			}
			if (empty_columns) {
				LOG_WARN() << "File " << _file_name << ": some columns were empty in line \"" << elements << "\"";
			}
			return true;
		} else {
			values.clear();
			return false;
		}
	}

	CSVFileReader::index_type CSVFileReader::read_data_row(std::vector<std::string>& row)
	{
		read_line();
		if (!_line.empty()) {
			try {
				_line_parser->parse(_line, row);
			} catch (DataException& e) {
				throw DataException(boost::str(boost::format("CSVFileReader: error parsing line %s: %s") % _line % e.what()));
			}
			return row.size();
		} else {
			row.clear();
			return 0;
		}
	}
	
	void CSVFileReader::to_data()
	{
		if (_has_names) {
			read_column_names(); // this moves us to the first data row, discarding the column names info
		} else {
			to_beginning(); // since there are no column names, data starts immediately
			_at_data = true;
		}
	}
	
	void CSVFileReader::to_beginning()
	{
		if (!_file.good()) {
			_file.clear(); // clear the possible "end of file" flag, otherwise seekg doesn't work
		}
		_file.seekg (0, _file.beg);
		next_line_idx_ = 0;
	}
	
	void CSVFileReader::read_line()
	{
		std::getline(_file, _line);
		// If running under Cygwin, the Windows end of line characters are not removed fully, we need to do it ourselves.
		if (_line.size() > 0 && _line[_line.size() - 1] == '\r') {
			_line = _line.substr(0, _line.size() - 1);
		}
		++next_line_idx_;
	}

	void CSVFileReader::read_elements()
	{
		read_line();
		if (!_line.empty()) {
			// split read line into elements
			try {
				_line_parser->parse(_line, _elements);
			} catch (DataException& e) {
				throw DataException(boost::str(boost::format("CSVFileReader: error parsing line %s: %s") % _line % e.what()));
			}
		} else {
			// no column names read
			_elements.resize(0);
		}
	}
	
	bool CSVFileReader::at_data() const
	{
		return _at_data;
	}

	CSVFileReader::index_type CSVFileReader::count_data_rows() {
		to_data();
		index_type cnt = 0;
		while (has_next_data_row()) {
			read_line();
			if (!_line.empty()) {
				++cnt;
			}
		}	
		to_data();
		return cnt;
	}

	CSVFileReader::index_type CSVFileReader::count_columns() {
		if (_has_names) {
			return read_column_names().size();
		} else {
			to_data();
			read_elements();
			const index_type nbr_cols = _elements.size();
			to_data(); // go back to beginning
			return nbr_cols;
		}
	}

    CSVFileReader::index_map_type CSVFileReader::map_names_to_indices(const std::vector<std::string>& names) {
        index_map_type map;
        const index_type n = names.size();
        for (index_type i = 0; i < n; ++i) {
            const std::string& name = names[i];
            if (map.find(name) == map.end()) {
                map[name] = i;
            } else {
                throw DataException("CSVFileReader: duplicate names");
            }
        }
        return map;
    }

	std::vector<CSVFileReader::index_type> CSVFileReader::convert_names_to_indices(const CSVFileReader::index_map_type& name_map, const std::vector<std::string>& names) {
		std::vector<index_type> indices(names.size());
		std::transform(names.begin(), names.end(), indices.begin(), [&name_map](const std::string& name) {
			const auto it = name_map.find(name);
			if (it != name_map.end()) {
				return it->second;
			} else {
				throw std::domain_error(boost::str(boost::format("CSVFileReader: cannot map name %s to index") % name));
			}
		});
		return indices;
	}

    CSVFileReader::index_type CSVFileReader::read_data_row(const index_map_type& headers, value_map_type& values) {
        read_elements();
        const index_type nelems = _elements.size();
        for (const auto& header_index: headers) {
            if (header_index.second < nelems) {
                values[header_index.first] = _elements[header_index.second];
            } else {
                values.erase(header_index.first);
            }
        }
        return nelems;
    }

	void CSVFileReader::check_column_present(const std::string& caller, const index_map_type& map, const std::string& name, const std::string& filename) {
		if (map.find(name) == map.end()) {
			throw DataException(boost::str(boost::format("%s: %s column required in file %s") % caller % name % filename));
		}
	}

	void CSVFileReader::check_value_present(const std::string& caller, const value_map_type& map, const std::string& name, const std::string& filename, bool allow_empty) {
		const auto iter = map.find(name);		
		if (iter == map.end() ||
			(!allow_empty && iter->second.empty())) {
			throw DataException(boost::str(boost::format("%s: value for column %s required in file %s") % caller % name % filename));
		}
	}

	CSVFileReader::LineIterator CSVFileReader::begin() {
		to_data();
		read_elements();
		return LineIterator(*this);
	}

	bool CSVFileReader::LineIterator::operator==(const LineIterator& other) const {
		assert(_reader == nullptr || other._reader == nullptr || _reader == other._reader);
		return _is_end == other._is_end;
	}

	CSVFileReader::LineIterator::LineIterator(CSVFileReader& reader)
		: _reader(&reader), _is_end(!reader.has_next_data_row()) {
	}

	CSVFileReader::LineIterator::LineIterator()
		: _reader(nullptr), _is_end(true) {
	}

	CSVFileReader::LineIterator::LineIterator(LineIterator&& other)
		: _reader(other._reader), _is_end(other._is_end) {
		other._reader = nullptr;
		other._is_end = true;
	}

	CSVFileReader::LineIterator::LineIterator(const LineIterator& other)
		: _reader(other._reader), _is_end(other._is_end) {		
	}

	void CSVFileReader::LineIterator::read_elements() {
		assert(_reader);
		_reader->read_elements();
	}

	CSVFileReader::LineIterator& operator++(CSVFileReader::LineIterator& iter) {
		if (!iter._is_end) {
			assert(iter._reader);
			if (iter._reader->has_next_data_row()) {
				iter.read_elements();
			} else {
				iter._is_end = true;
			}
		}
        return iter;
	}

	PostIncrementProxy<CSVFileReader::LineIterator::value_type> CSVFileReader::LineIterator::operator++(int) {
		PostIncrementProxy<value_type> v(this->operator*());
		++(*this);
		return v;
	}

	CSVFileReader::StringIterator::StringIterator(StringIterator&& other)
		: _line_iterator(std::move(other._line_iterator)),
		_elem_idx(other._elem_idx) {
	}

	CSVFileReader::StringIterator::StringIterator(const StringIterator& other)
		: _line_iterator(other._line_iterator),
		_elem_idx(other._elem_idx) {
	}

	CSVFileReader::StringIterator::value_type& CSVFileReader::StringIterator::operator*() const {
		const auto& elems = *_line_iterator;
		if (_elem_idx < elems.size()) {
			return (*_line_iterator)[_elem_idx];
		} else {			
			const auto& reader = *(_line_iterator._reader);
			throw DataException(boost::str(boost::format("CSVFileReader: no element %d in line %s (next line index %d) in file %s") % _elem_idx % elems % reader.next_line_idx_ % reader._file_name));
		}
	}

	CSVFileReader::StringIterator::value_type* CSVFileReader::StringIterator::operator->() const {
		return &this->operator*();
	}

	CSVFileReader::StringIterator& operator++(CSVFileReader::StringIterator& iter) {
		++iter._line_iterator;
        return iter;
	}

	PostIncrementProxy<CSVFileReader::StringIterator::value_type> CSVFileReader::StringIterator::operator++(int) {
		PostIncrementProxy<value_type> v(this->operator*());
		++(*this);
		return v;
	}

	bool CSVFileReader::StringIterator::operator==(const StringIterator& other) const {
		return _line_iterator == other._line_iterator && _elem_idx == other._elem_idx;
	}

	CSVFileReader::StringIterator::StringIterator(LineIterator&& line_iterator, size_t elem_idx)
		: _line_iterator(std::move(line_iterator)),
		_elem_idx(elem_idx) {}

	

	CSVFileReader::MultiDoubleIterator::MultiDoubleIterator(MultiDoubleIterator&& other)
		: _line_iterator(std::move(other._line_iterator)),
		_indices(std::move(other._indices)),
		_values(std::move(other._values)) {
	}

	CSVFileReader::MultiDoubleIterator::MultiDoubleIterator(const MultiDoubleIterator& other)
		: _line_iterator(other._line_iterator),
		_indices(other._indices),
		_values(other._values) {
	}

	CSVFileReader::MultiDoubleIterator::MultiDoubleIterator(CSVFileReader::LineIterator&& line_iterator, const std::vector<index_type>& indices, bool fill_with_nans)
		: _line_iterator(std::move(line_iterator)),
		_indices(indices),
		_fill_with_nans(fill_with_nans) {
		convert();
	}

	CSVFileReader::MultiDoubleIterator& operator++(CSVFileReader::MultiDoubleIterator& iter) {
		++iter._line_iterator;
		iter.convert();
		return iter;
	}

	PostIncrementProxy<CSVFileReader::MultiDoubleIterator::value_type> CSVFileReader::MultiDoubleIterator::operator++(int) {
		PostIncrementProxy<value_type> v(this->operator*());
		++(*this);
		return v;
	}

	bool CSVFileReader::MultiDoubleIterator::operator==(const MultiDoubleIterator& other) const {
		return _line_iterator == other._line_iterator && _fill_with_nans == other._fill_with_nans && _indices == other._indices;
	}
	
	void CSVFileReader::MultiDoubleIterator::convert() {
		if (!_line_iterator.is_end()) {
			_line_iterator._reader->read_data_row(*_line_iterator, _indices, _fill_with_nans, _values);
		}
	}

	CSVFileReader::StateKeeper::StateKeeper(CSVFileReader& reader)
		: PositionKeeperIn(reader._file), reader_(reader), next_line_idx_(reader.next_line_idx_), at_data_(reader._at_data) {}

	CSVFileReader::StateKeeper::~StateKeeper() {
		reader_.next_line_idx_ = next_line_idx_;
		reader_._at_data = at_data_;
	}
}

// Author: Agnieszka Werpachowska, 2014
