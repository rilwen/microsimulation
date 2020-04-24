#ifndef __AVERISERA_PAIR_ITERATOR_HPP
#define __AVERISERA_PAIR_ITERATOR_HPP

#include <iterator>
#include <utility>

namespace averisera {
    /** Iterator which is a pair of other iterators incremented/decremented together. NOT exception and thread safe. */
    template <class I1, class I2> class PairConstIterator: public std::iterator<std::input_iterator_tag, std::pair<typename std::iterator_traits<I1>::value_type, typename std::iterator_traits<I2>::value_type> > {
    public:
        typedef typename std::iterator_traits<I1>::value_type v1_t;
        typedef typename std::iterator_traits<I2>::value_type v2_t;
        typedef std::pair<v1_t, v2_t> value_type;

        PairConstIterator() = default;
        
        explicit PairConstIterator(I1 i1, I2 i2)
            : _i1(i1), _i2(i2) {
        }        

        PairConstIterator(const PairConstIterator<I1, I2>& other) = default;

        PairConstIterator<I1, I2>& operator=(const PairConstIterator<I1, I2>& other) = default;

        /** Checks the first iterator only (assumes this and other refer to the same container) */
        bool operator==(const PairConstIterator<I1, I2>& other) const {
            return _i1 == other._i1;
        }

        /** Checks the first iterator only (assumes this and other refer to the same container) */
        bool operator<(const PairConstIterator<I1, I2>& other) const {
            return _i1 < other._i1;
        }

        /** Checks the first iterator only (assumes this and other refer to the same container) */
        bool operator<=(const PairConstIterator<I1, I2>& other) const {
            return _i1 <= other._i1;
        }

        /** Checks the first iterator only (assumes this and other refer to the same container) */
        bool operator!=(const PairConstIterator<I1, I2>& other) const {
            return _i1 != other._i1;
        }

        PairConstIterator<I1, I2>& operator++() {
            // not exception safe
            ++_i1;
            ++_i2;
            return *this;
        }

        PairConstIterator<I1, I2>& operator--() {
            // not exception safe
            --_i1;
            --_i2;
            return *this;
        }

        const value_type& operator*() const {
            _val = std::make_pair(*_i1, *_i2);
            return _val;
        }
        
        const I1& first() const {
            return _i1;
        }

        const I2& second() const {
            return _i2;
        }
    private:
        I1 _i1;
        I2 _i2;
        mutable value_type _val;
    };

    template <class I1, class I2> PairConstIterator<I1, I2> make_pair_iterator(I1 i1, I2 i2) {
        return PairConstIterator<I1, I2>(i1, i2);
    }
}

#endif
