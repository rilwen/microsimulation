/*
  (C) Averisera Ltd 2014
  Author: Agnieszka Werpachowska
*/
#ifndef __AVERISERA_SORTING_H
#define __AVERISERA_SORTING_H

#include <vector>
#include <set>
#include <stdexcept>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/exception.hpp>

namespace averisera {
	/** @brief Utility methods for sorting */
	namespace Sorting {
		template <class T> using index_value_pair = std::pair<size_t, T> ;

		// Sort (index, value) pairs by value in ascending order
		template <class T> void sort_index_value(std::vector<index_value_pair<T>>& index_value_pairs) {
			std::sort(index_value_pairs.begin(), index_value_pairs.end(), [](const index_value_pair<T>& l, const index_value_pair<T>& r) { return l.second < r.second; });
		}

		/** Sort pointers to objects by object value in ascending order */
		template <class T> void sort_pointers(std::vector<T*>& pointers) {
			std::sort(pointers.begin(), pointers.end(), [](T* l, T* r) { return *l < *r; });
		}

		/** Topological sort of a vector of values on which we have a non-transitive "less than"
	  relation defined by the cmp functor.
	  @tparam T Copyable value
	  @param[in,out] values Vector to be sorted
	  @param[in] cmp Functor describes the relation between sorted values. For a, b in values,
	  cmp(a, b) < 0 if a precedes b, > 0 if b preceds a and == 0 if there is no relation.
	  @throws std::runtime_error If relation graph has cycles
		*/
		template <class T, class C> void topological_sort(std::vector<T>& values, C cmp) {
			typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> Graph;
			typedef std::pair<int, int> Edge;
			const size_t n = values.size();
			int i1 = 0;
			std::vector<Edge> edges;
			for (auto it1 = values.begin(); it1 != values.end(); ++it1, ++i1) {
				assert(static_cast<size_t>(i1) < n);
				const auto& v1 = *it1;
				int i2 = 0;
				for (auto it2 = values.begin(); it2 != it1; ++it2, ++i2) {
					assert(i2 < i1);
					const int rel = cmp(v1, *it2);
					if (rel != 0) {
						if (rel > 0) {
							edges.push_back(Edge(i1, i2)); // reverse the relation to obtain ascending order from boost topological sort
						} else {
							edges.push_back(Edge(i2, i1));
						}
					}
				}
			}
			Graph g(edges.begin(), edges.end(), n);
			std::vector<size_t> sorted_vertices(n, -1);
			try {
				boost::topological_sort(g, sorted_vertices.begin());
			} catch (boost::not_a_dag& e) {
				throw std::runtime_error(std::string("Operator: bad relation graph: ") + e.what());
			}
			const std::vector<T> copy(values);
			for (size_t i = 0; i < n; ++i) {
				const size_t vert_idx = sorted_vertices[i];
				assert(vert_idx >= 0);
				values[i] = copy[vert_idx];
			}
		}
	}
}

#endif
