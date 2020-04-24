#include "ethnicity.hpp"
#include <boost/format.hpp>
#include <ostream>
#include <stdexcept>

namespace averisera {
	namespace microsim {
		namespace Ethnicity {
			index_set_type index_range_to_set(const index_range_type& range) {
				index_set_type set;
				for (auto idx = range.begin(); idx <= range.end(); ++idx) {
					set.insert(idx);
				}
				return set;
			}

			std::vector<index_set_type> index_ranges_to_sets(const std::vector<index_range_type>& ranges, const group_index_type size) {
				std::vector<index_set_type> sets(ranges.size());
				std::transform(ranges.begin(), ranges.end(), sets.begin(), index_range_to_set);
				index_set_type all_specified_groups;
				size_t all_others_idx = sets.size();
				size_t idx = 0;
				for (auto& set : sets) {
					if (set.count(size)) {
						if (set.size() == 1) {
							// found "all others" set
							if (all_others_idx == sets.size()) {
								all_others_idx = idx;
								++idx;
								continue;
							} else {
								// we've been here before!
								throw DataException("Ethnicity: duplicate ALL_OTHERS range");
							}
						}
					}
					for (const auto& i : set) {
						if (i > size) {
							throw DataException("Ethnicity: ethnic group index value too large");
						}
						all_specified_groups.insert(i);
					}					
					++idx;
				}
				if (all_others_idx < sets.size()) {
					index_set_type all_others;
					for (group_index_type i = get_first_group_index(); i < size; ++i) {
						if (!all_specified_groups.count(i)) {
							all_others.insert(i);
						}
					}
					sets[all_others_idx] = std::move(all_others);
				}
				return sets;
			}

			const char* const EMPTY = "";

			index_range_type placeholder(const char*) {
				throw std::logic_error("Ethnicity: placeholder conversion used");
			}

			IndexConversions::IndexConversions()
				: class_name_(EMPTY),
				size_(0),
				index_range_from_string_(placeholder),
				index_range_all_(0, 0),
				index_range_all_others_(0, 0) {
			}

			IndexConversions::IndexConversions(IndexConversions&& other)
				: class_name_(other.class_name_),
				size_(other.size_),
				names_(std::move(other.names_)),
				indices_(std::move(other.indices_)),
				index_range_from_string_(std::move(other.index_range_from_string_)),
				index_range_all_(std::move(other.index_range_all_)),
				index_range_all_others_(std::move(other.index_range_all_others_)) {
				other.class_name_ = EMPTY;
				other.size_ = 0;
				other.index_range_from_string_ = placeholder;
				other.index_range_all_ = index_range_type(0, 0);
				other.index_range_all_others_ = index_range_type(0, 0);
			}

			std::ostream& operator<<(std::ostream& os, const IndexConversions& ic) {
				os << ic.classification_name() << ": " << ic.names_;
				return os;
			}
		}
	}
}
