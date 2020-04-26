// (C) Averisera Ltd 2014-2020
#pragma once
#include "dates.hpp"
#include "utils.hpp"
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace averisera {
    /** Utility functions dealing with STL containers and pointers: getting values and formatted output. */
    namespace StlUtils {
        template <class T> std::set<T> set_union(const std::set<T>& first, const std::set<T>& second) {
            if (second.size() <= first.size()) {
                std::set<T> result(first);
                result.insert(second.begin(), second.end());
                return result;
            } else {
                return set_union(second, first);
            }
        }

        template <class T> std::unordered_set<T> set_union(const std::unordered_set<T>& first, const std::unordered_set<T>& second) {
            if (second.size() <= first.size()) {
                std::unordered_set<T> result(first);
                result.insert(second.begin(), second.end());
                return result;
            } else {
                return set_union(second, first);
            }
        }

		/** Like Python "get" method in a dictionary */
		template <class K, class V> const V& get(const std::map<K, V>& map, const K& key, const V& default_value) {
			const auto iter = map.find(key);
			if (iter != map.end()) {
				return iter->second;
			} else {
				return default_value;
			}
		}

		/** Like Python "get" method in a dictionary */
		template <class K, class V, class H> const V& get(const std::unordered_map<K, V, H>& map, const K& key, const V& default_value) {
			const auto iter = map.find(key);
			if (iter != map.end()) {
				return iter->second;
			} else {
				return default_value;
			}
		}

		/** Like Python "get" method in a dictionary, but modifies the map to insert the copy of the default element in it if it's not present */
		template <class K, class V, class H> V& get(std::unordered_map<K, V, H>& map, const K& key, const V& default_value) {
			const auto iter = map.find(key);
			if (iter != map.end()) {
				return iter->second;
			} else {
				return map.insert(iter, std::make_pair(key, default_value))->second;
			}
		}

		/** Check if map contains key */
		template <class K, class V> bool contains(const std::map<K, V>& map, const K& key) {
			return map.find(key) != map.end();
		}

		/** Check if unordered map contains key */
		template <class K, class V> bool contains(const std::unordered_map<K, V>& map, const K& key) {
			return map.find(key) != map.end();
		}

        /** Check if set contains key */
        template <class K> bool contains(const std::set<K>& set, const K& key) {
            return set.find(key) != set.end();
        }

        /** Check if unordered set contains key */
        template <class K> bool contains(const std::unordered_set<K>& set, const K& key) {
            return set.find(key) != set.end();
        }

        template <class C> void print_stl_container(std::ostream& stream, const C& container) {
            stream << "[";
            for (auto it = container.begin(); it != container.end(); ++it) {
                if (it != container.begin()) {
                    stream << ", ";
                }
                stream << *it;
            }
            stream << "]";
        }

		template <class T, class C> void print_stl_container_casting(std::ostream& stream, const C& container) {
			stream << "[";
			for (auto it = container.begin(); it != container.end(); ++it) {
				if (it != container.begin()) {
					stream << ", ";
				}
				stream << static_cast<T>(*it);
			}
			stream << "]";
		}

		/** Merge sorted vector with unique values */
		template <class T> std::vector<T> merge_sorted_vectors(const std::vector<T>& vec1, const std::vector<T>& vec2) {
			std::vector<T> vec;
			vec.reserve(vec1.size() + vec2.size());
			std::set_union(vec1.begin(), vec1.end(), vec2.begin(), vec2.end(), std::back_inserter(vec));
			vec.shrink_to_fit();
			return vec;
		}

        /** Use this to create unordered sets and maps with enum class keys which work in GCC */
        struct EnumClassHash {
            template <class T> size_t operator()(T v) const {
                return static_cast<size_t>(v);
            }
        };
    }
}

namespace std {
	template <class T> std::ostream& operator<<(std::ostream& stream, const std::vector<T>& vec) {
        averisera::StlUtils::print_stl_container_casting<typename averisera::Utils::printable_type<T>::type>(stream, vec);
        return stream;
    }
    
	template <class T, size_t N> std::ostream& operator<<(std::ostream& stream, const std::array<T, N>& vec) {
		averisera::StlUtils::print_stl_container_casting<typename averisera::Utils::printable_type<T>::type>(stream, vec);
		return stream;
	}
	
	template <class T> std::ostream& operator<<(std::ostream& stream, const std::set<T>& set) {
        averisera::StlUtils::print_stl_container_casting<typename averisera::Utils::printable_type<T>::type>(stream, set);
        return stream;
    }

    template <class T> std::ostream& operator<<(std::ostream& stream, const std::unordered_set<T>& uset) {
        averisera::StlUtils::print_stl_container_casting<typename averisera::Utils::printable_type<T>::type>(stream, uset);
        return stream;
    }

	template <class K, class V> std::ostream& operator<<(std::ostream& stream, const std::unordered_map<K, V>& umap) {
        averisera::StlUtils::print_stl_container(stream, umap);
        return stream;
    }

	template <class K, class V> std::ostream& operator<<(std::ostream& stream, const std::map<K, V>& umap) {
		averisera::StlUtils::print_stl_container(stream, umap);
		return stream;
	}

    template <class A, class B> std::ostream& operator<<(std::ostream& stream, const std::pair<A, B>& pair) {
        stream << "<" << static_cast<typename averisera::Utils::printable_type<A>::type>(pair.first) << ", " << static_cast<typename averisera::Utils::printable_type<B>::type>(pair.second) << ">";
        return stream;
    }

    template <class T> std::ostream& operator<<(std::ostream& stream, const std::unique_ptr<T>& ptr) {
        stream << "unique_ptr[" << ptr.get() << "]";
        return stream;
    }

    template <class T> std::ostream& operator<<(std::ostream& stream, const std::shared_ptr<T>& ptr) {
        stream << "shared_ptr[" << ptr.get() << "]";
        return stream;
    }

    template <class T> std::ostream& operator<<(std::ostream& stream, const std::weak_ptr<T>& ptr) {
        stream << "weak_ptr[" << ptr.get() << "]";
        return stream;
    }

	template <class A, class B> std::ostream& operator<<(std::ostream& stream, const std::tuple<A, B>& tuple) {
		stream << "(" << static_cast<typename averisera::Utils::printable_type<A>::type>(std::get<0>(tuple)) << ", " << static_cast<typename averisera::Utils::printable_type<B>::type>(std::get<1>(tuple)) << ")";
		return stream;
	}

	template <class A, class B, class C> std::ostream& operator<<(std::ostream& stream, const std::tuple<A, B, C>& tuple) {
		stream << "(" << static_cast<typename averisera::Utils::printable_type<A>::type>(std::get<0>(tuple)) << ", " << static_cast<typename averisera::Utils::printable_type<B>::type>(std::get<1>(tuple)) << ", " << static_cast<typename averisera::Utils::printable_type<C>::type>(std::get<2>(tuple)) << ")";
		return stream;
	}

	template <class L, class R> struct hash<std::pair<L, R>> {
		typedef std::pair<L, R> argument_type;
		typedef size_t result_type;
		result_type operator()(const argument_type& arg) const {
			return hash<L>()(arg.first) + 31 * hash<R>()(arg.second);
		}
	};
}

