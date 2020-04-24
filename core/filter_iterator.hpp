#pragma once
#include "safe_iterator.hpp"
#include <iterator>

namespace averisera {

	/** InputIterator which hides all values pointed to by I for which P does not return true. 
	@tparam I InputIterator
	@tparam P Predicate with "bool operator(const I::value_type& x) const" method.
	@tparam D Predicate which checks if I is dereferencable
	*/
	template <class I, class P, class D = IsDereferencable<I>> class FilterIterator: public std::iterator<std::input_iterator_tag, typename I::value_type> {
	public:
		/** @param iter Unfiltered iterator
		@param end End-of-range iterator (required so that we can safely call pred(*iter))
		@param pred Predicate 
		*/
		FilterIterator(I iter, P pred, D deref)
			: _iter(iter), _pred(pred), _deref(deref) {}
        
		FilterIterator(const FilterIterator<I, P, D>& other) = default;
        
		FilterIterator<I, P, D>& operator=(const FilterIterator<I, P, D>& other) = default;
        
		friend FilterIterator<I, P, D>& operator++(FilterIterator<I, P, D>& iter) {
			++iter._iter;
			iter.seek();
			return iter;
		}
        
		typename FilterIterator<I, P, D>::value_type& operator*() {
			seek();
			return *_iter;
		}
        
		typename FilterIterator<I, P, D>::value_type* operator->() {
			seek();
			return _iter.operator->();
		}
        
		bool operator!=(const FilterIterator<I, P, D>& other) const {
			return _iter != other._iter;
		}		
	private:
		I _iter;
		P _pred;
		D _deref;
		void seek() {
			while (_deref(_iter) && (!_pred(*_iter))) {
				++_iter;
			}
		}
	};

	template <class I, class P> FilterIterator<I, P> make_filter_iterator(I iter, I end, P pred) {
		return FilterIterator<I, P>(iter, pred, IsDereferencable<I>(end));
	}

	template <class I, class P, class D> FilterIterator<I, P, D> make_filter_iterator(I iter, P pred, D deref) {
		return FilterIterator<I, P, D>(iter, pred, deref);
	}
}
