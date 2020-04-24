/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "multi_index.hpp"
#include "math_utils.hpp"
#include <cmath>
#include <cstddef>
#include <cassert>

namespace averisera {
	MultiIndex::MultiIndex(unsigned int dim, size_t size)
		: _dim(dim), _size(size), _flat_size(MathUtils::pow(size, dim)), _flat(0), _indices(dim, 0)
	{}

	MultiIndex& MultiIndex::operator++() {
		const auto idx_end = _indices.end();
		for (auto idx_iter = _indices.begin(); idx_iter != idx_end; ++idx_iter) {
			++(*idx_iter);
			if (*idx_iter < _size) {
				break;
			} else {
				// go back to zero and increment next index
				*idx_iter = 0;
			}
		}
		++_flat;
		return *this;
	}

	void MultiIndex::reset() {
		_flat = 0;
		_indices.assign(_dim, 0);
	}

	MultiIndex& MultiIndex::operator=(size_t flat) {
		assert(flat < _flat_size);
		_flat = flat;
		decompose(flat, _size, _dim, _indices);
		return *this;
	}

	void MultiIndex::set_index(size_t i, size_t value) {
		set_index(i, MathUtils::pow(_size, static_cast<unsigned int>(i)), value);
	}

	void MultiIndex::set_index(const size_t i, const size_t step_size, const size_t value) {
		assert(value < _size);
		const ptrdiff_t delta = value - _indices[i];
		_flat += step_size * delta;
		if (_flat >= _flat_size) {
			_flat -= _flat_size;
		}
		_indices[i] = value;
	}

	void MultiIndex::decompose(size_t flat_index, const size_t size, const size_t dim, std::vector<size_t>& indices) {
		for (size_t i = 0; i < dim; ++i) {
			const size_t a = flat_index / size;
			indices[i] = flat_index - a * size;
			flat_index = a;
		}
	}

	size_t MultiIndex::flatten(size_t size, const std::vector<size_t>& indices) {
		size_t flat_index = 0;
		for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
			flat_index = size * flat_index + *it;
		}
		return flat_index;
	}

	MultiIndex::Index MultiIndex::index(size_t pos) {
		assert(pos < _dim);
		return Index(this, pos);
	}

	MultiIndex::Index::Index(MultiIndex* mi, size_t pos)
		: _pos(pos), _mi(mi)
	{
		assert(mi != nullptr);
		assert(pos < mi->_dim);
		_step_size = MathUtils::pow(mi->_size, static_cast<unsigned int>(pos));
	}

	MultiIndex::Index& MultiIndex::Index::operator++() {
		assert(has_next());
		++_mi->_indices[_pos];
		_mi->_flat += _step_size;
		return *this;
	}

	MultiIndex::Index& MultiIndex::Index::operator=(size_t value) {
		_mi->set_index(_pos, _step_size, value);
		return *this;
	}
}
