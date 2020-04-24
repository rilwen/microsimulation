/*
(C) Averisera Ltd 2014
Author: Agnieszka Werpachowska
*/
#include "multi_index_multisize.hpp"
#include "math_utils.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <stdexcept>

namespace averisera {
    MultiIndexMultisize::MultiIndexMultisize()
        : MultiIndexMultisize(std::vector<size_t>()) {
        _flat_size = 0;
    }
    
	MultiIndexMultisize::MultiIndexMultisize(const std::vector<size_t>& sizes)
		: _dim(static_cast<unsigned int>(sizes.size())),
          _sizes(sizes),
          _flat_size(MathUtils::cum_prod(sizes.begin(), sizes.end(), static_cast<size_t>(1))),
          _flat(0),
          _indices(_dim, 0) {
        validate();
    }

    MultiIndexMultisize::MultiIndexMultisize(std::vector<size_t>&& sizes)
		: _dim(static_cast<unsigned int>(sizes.size())), 
          _sizes(std::move(sizes)), // initialisation order is important because of std::move
          _flat_size(MathUtils::cum_prod<size_t>(_sizes.begin(), _sizes.end(), 1)),
          _flat(0),
          _indices(_dim, 0) {
        sizes.resize(0);
        validate();
    }

    MultiIndexMultisize& MultiIndexMultisize::operator=(MultiIndexMultisize&& other) {
        if (this != &other) {
            _dim = other._dim;
            _sizes = std::move(other._sizes);
            _flat_size = other._flat_size;
            _flat = other._flat;
            _indices = std::move(other._indices);
            other._sizes.resize(0);
            other._dim = 0;
            other._flat_size = 0;
            other._indices.resize(0);
        }
        return *this;
    }

	MultiIndexMultisize& MultiIndexMultisize::operator++() {
		const auto idx_end = _indices.end();
        unsigned int pos = 0;
		for (auto idx_iter = _indices.begin(); idx_iter != idx_end; ++idx_iter, ++pos) {
			++(*idx_iter);
			if (*idx_iter < _sizes[pos]) {
				break;
			} else {
				// go back to zero and increment next index
				*idx_iter = 0;
			}
		}
		++_flat;
		return *this;
	}

	void MultiIndexMultisize::reset() {
		_flat = 0;
		_indices.assign(_dim, 0);
	}

	MultiIndexMultisize& MultiIndexMultisize::operator=(size_t flat) {
		assert(flat < _flat_size);
		_flat = flat;
		decompose(flat, _sizes, _indices);
		return *this;
	}

	void MultiIndexMultisize::set_index(size_t i, size_t value) {
        const size_t step_size = MathUtils::cum_prod<size_t>(_sizes.begin(), _sizes.begin() + i, 1u);
		set_index(i, step_size, value);
	}

	void MultiIndexMultisize::set_index(const size_t i, const size_t step_size, const size_t value) {
		assert(value < _sizes.at(i));
		const ptrdiff_t delta = value - _indices[i];
		_flat += step_size * delta;
		if (_flat >= _flat_size) {
			_flat -= _flat_size;
		}
		_indices[i] = value;
	}

	void MultiIndexMultisize::decompose(size_t flat_index, const std::vector<size_t>& sizes, std::vector<size_t>& indices) {
        decompose_impl<size_t>(flat_index, sizes, indices);
	}    

	size_t MultiIndexMultisize::flatten(const std::vector<size_t>& sizes, const std::vector<size_t>& indices) {
		size_t flat_index = 0;
        auto size_it = sizes.rbegin();
		for (auto it = indices.rbegin(); it != indices.rend(); ++it, ++size_it) {
            assert(size_it != sizes.rend());
			flat_index = (*size_it) * flat_index + *it;
		}
		return flat_index;
	}

	MultiIndexMultisize::Index MultiIndexMultisize::index(size_t pos) {
		assert(pos < _dim);
		return Index(this, pos);
	}

    void MultiIndexMultisize::validate() const {
        if (std::any_of(_sizes.begin(), _sizes.end(), [](size_t size){ return size == 0; })) {
            throw std::domain_error("MultiIndexMultisize: one or more sizes is 0");
        }
    }

	MultiIndexMultisize::Index::Index(MultiIndexMultisize* mi, size_t pos)
		: _pos(pos), _mi(mi)
	{
		assert(mi != nullptr);
		assert(pos < mi->_dim);
		_step_size = MathUtils::cum_prod<size_t>(mi->_sizes.begin(), mi->_sizes.begin() + pos, 1);
	}

	MultiIndexMultisize::Index& MultiIndexMultisize::Index::operator++() {
		assert(has_next());
		++_mi->_indices[_pos];
		_mi->_flat += _step_size;
		return *this;
	}

	MultiIndexMultisize::Index& MultiIndexMultisize::Index::operator=(size_t value) {
		_mi->set_index(_pos, _step_size, value);
		return *this;
	}
}
