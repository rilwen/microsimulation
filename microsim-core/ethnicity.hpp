// (C) Averisera Ltd 2014-2020
#pragma once
#include "core/data_exception.hpp"
#include "core/math_utils.hpp"
#include "core/preconditions.hpp"
#include "core/range.hpp"
#include "core/stl_utils.hpp"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iosfwd>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace averisera {
	namespace microsim {
		/** Methods operating on ethnic group classifications

		We assume that the group enum class type is associated with integer values 0, 1, 2, ..., SIZE - 1
		*/
		namespace Ethnicity {
			template <class C> using group_type = typename C::Group;
			typedef uint8_t group_index_type;
            const group_index_type MAX_SIZE = 128; /**< Maximum number of ethnicity groups. */

            template <class C> constexpr group_index_type get_size() {
				return static_cast<group_index_type>(group_type<C>::SIZE);
			}

			/** Return a static C string */
			template <class C> constexpr const char* get_name(group_index_type idx) {
				return C::NAMES[idx];
			}

			/** Return a static C string */
			template <class C> constexpr const char* get_name(group_type<C> grp) {
				return get_name<C>(static_cast<group_index_type>(grp));
			}

			/** Return a static C string */
			template <class C> constexpr const char* const& get_classification_name() {
				return C::CLASSIFICATION_NAME;
			}

			template <class C> group_index_type get_group_index(const char* name) {
				group_index_type i = 0;
				for (const char* str : C::NAMES) {
					if (!strcmp(str, name)) {
                        assert(i < MAX_SIZE);
						return i;
					} else {
						++i;
					}
				}
				return get_size<C>();
			}

			template <class C> group_type<C> get_group(const char* name) {
				return static_cast<group_type<C>>(get_group_index<C>(name));
			}

			constexpr group_index_type get_first_group_index() {
				return 0;
			}

			template <class C> constexpr group_type<C> get_first_group() {
				return static_cast<group_type<C>>(get_first_group_index());
			}

			template <class C> constexpr group_index_type get_last_group_index() {
				return static_cast<group_index_type>(C::SIZE - 1);
			}

			template <class C> constexpr group_type<C> get_last_group() {
				return static_cast<group_type<C>>(get_last_group_index<C>());
			}

			/** Execute f(c) for c in [begin, end] */
			template <class C, class F> void iterate_over(group_type<C> begin, group_type<C> end, F f) {
				const size_t i1 = static_cast<size_t>(end);
				for (size_t i = static_cast<size_t>(begin); i <= i1; ++i) {
					f(static_cast<typename C::Group>(i));
				}
			}

			template <class C> using range_type = averisera::Range<group_type<C>>;

			typedef averisera::Range<group_index_type> index_range_type;

			template <class C> constexpr range_type<C> get_range_all() {
				return range_type<C>(get_first_group<C>(), get_last_group<C>());
			}

			template <class C> constexpr index_range_type get_index_range_all() {
				return index_range_type(get_first_group_index(), get_last_group_index<C>());
			}

			template <class C> constexpr range_type<C> get_range_all_others() {
				return range_type<C>(C::Group::SIZE, C::Group::SIZE);
			}

			template <class C> constexpr index_range_type get_index_range_all_others() {
				return index_range_type(static_cast<group_index_type>(C::SIZE), static_cast<group_index_type>(C::SIZE));
			}

			template <class C> index_range_type convert_range(const range_type<C>& range) {
				return index_range_type(static_cast<group_index_type>(range.begin()), static_cast<group_index_type>(range.end()));
			}

			template <class C> range_type<C> convert_range(const index_range_type& range) {
				return range_type<C>(static_cast<group_type<C>>(range.begin()), static_cast<group_type<C>>(range.end()));
			}

			const char* const RANGE_ALL = "ALL"; /**< Name for the range including all groups */			
			const char* const RANGE_ALL_OTHERS = "ALL_OTHERS"; /** Name for the "all other groups" quasi-range. We encode it as [SIZE, SIZE] range */

			template <class C> index_range_type index_range_from_string(const char* str) {
				if (!strcmp(str, RANGE_ALL)) {
					return get_index_range_all<C>();
				} else if (!strcmp(str, RANGE_ALL_OTHERS)) {
					return get_index_range_all_others<C>();
				} else {
					static const group_index_type last_group_index = get_last_group_index<C>();
					return index_range_type::from_string(str, [](const std::string& str) { return get_group_index<C>(str.c_str()); }, nullptr, &last_group_index);
				}
			}

			template <class C> range_type<C> range_from_string(const char* str) {				
				return convert_range<C>(index_range_from_string<C>(str));
			}

			template <class C> using set_type = std::unordered_set<group_type<C>, StlUtils::EnumClassHash>;

			/* Simple conversion of [g1, g2] range to a set of all groups in this range */
			template <class C> set_type<C> range_to_set(const range_type<C>& range) {
				set_type<C> set;
				iterate_over<C>(range.begin(), range.end(), [&set](group_type<C> grp) {
					set.insert(grp);
				});
				return set;
			}

			typedef std::unordered_set<group_index_type> index_set_type;

			/* Simple conversion of [i1, i2] range to a set of all group indices in this range */
			index_set_type index_range_to_set(const index_range_type& range);

			/** Convert ranges into sets. If [SIZE, SIZE] range is found and is unique, it is replaced by a set containing all
			values not found in other ranges. It is an error to provide [SIZE, SIZE] range more than once. 
			@throw DataException */
			template <class C> std::vector<set_type<C>> ranges_to_sets(const std::vector<range_type<C>>& ranges) {
				std::vector<set_type<C>> sets(ranges.size());
				std::transform(ranges.begin(), ranges.end(), sets.begin(), range_to_set<C>);
				set_type<C> all_specified_groups;
				size_t all_others_idx = sets.size();
				size_t idx = 0;
				for (auto& set : sets) {
					if (set.count(C::Group::SIZE)) {
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
					all_specified_groups.insert(set.begin(), set.end());
					++idx;
				}
				if (all_others_idx < sets.size()) {
					set_type<C> all_others;
					iterate_over<C>(get_first_group<C>(), get_last_group<C>(), [&sets, &all_specified_groups, &all_others](group_type<C> grp) {
						if (!all_specified_groups.count(grp)) {
							all_others.insert(grp);
						}
					});
					sets[all_others_idx] = std::move(all_others);
				}
				return sets;
			}

			/** Convert ranges into sets. If [size, size] range is found and is unique, it is replaced by a set containing all
			values not found in other ranges. It is an error to provide [size, size] range more than once. No range boundary values larger than size are permitted.
			@throw DataException */
			std::vector<index_set_type> index_ranges_to_sets(const std::vector<index_range_type>& ranges, group_index_type size);

			/** Index-based conversion functions */
			class IndexConversions {
			public:
				typedef group_index_type index_type;
				typedef Ethnicity::index_range_type index_range_type;

				/** Default conversion which does not convert anything */
				IndexConversions();

				IndexConversions(IndexConversions&& other);

				IndexConversions(const IndexConversions& other) = default;

				IndexConversions& operator=(const IndexConversions& other) = default;

				template <class C> static IndexConversions build() {
					IndexConversions ic;
					ic.class_name_ = get_classification_name<C>();
					ic.size_ = MathUtils::safe_cast<group_index_type>(get_size<C>());
					ic.names_.resize(ic.size_);					
					for (index_type i = 0; i < ic.size_; ++i) {
						ic.names_[i] = get_name<C>(i);
						ic.indices_[ic.names_[i]] = i;
					}
					ic.index_range_from_string_ = Ethnicity::index_range_from_string<C>;
					ic.index_range_all_ = Ethnicity::get_index_range_all<C>();
					ic.index_range_all_others_ = Ethnicity::get_index_range_all_others<C>();
					return ic;
				}

				const char* classification_name() const {
					return class_name_;
				}

				/** Number of ethnic groups */
				index_type size() const {
					return size_;
				}

				/** Name of the idx-th ethnic group */
				const std::string& name(index_type idx) const {
					assert(idx < size_);
					return names_[idx];
				}

				/** Return the index of group name 
				or size() if name does not match (if check == false) or throw std::domain_error if check == true
				 */
				index_type index(const std::string& name, bool check = true) const {
					const auto it = indices_.find(name);
					if (it != indices_.end()) {
						return it->second;
					} else {
						if (check) {
							throw std::domain_error(boost::str(boost::format("Ethnicity::IndexConversions: unknown group name %s in classification %s") % name % class_name_));
						} else {
							return size();
						}
					}
				}

                /** Return first valid index. Call only if size() > 0 */
				index_type first_index() const {
                    assert(size_ > 0);
					return 0;
				}

                /** Return last valid index. Call only if size() > 0 */
				index_type last_index() const {
                    assert(size_ > 0);
					return static_cast<index_type>(size_ - 1);
				}

				index_range_type index_range_from_string(const std::string& str) const {
					return index_range_from_string_(str.c_str());
				}

				const index_range_type& index_range_all() const {
					return index_range_all_;
				}

				const index_range_type& index_range_all_others() const {
					return index_range_all_others_;
				}

				/** Convert ranges into sets. If [size(), size()] range is found and is unique, it is replaced by a set containing all
				values not found in other ranges. It is an error to provide [size(), size()] range more than once. No range boundary values larger than size are permitted.
				@throw DataException */
				std::vector<index_set_type> index_ranges_to_sets(const std::vector<index_range_type>& ranges) const {
					return Ethnicity::index_ranges_to_sets(ranges, size());
				}

				friend std::ostream& operator<<(std::ostream& os, const IndexConversions& ic);
			private:								
				const char* class_name_;
				index_type size_;
				std::vector<std::string> names_;
				std::map<std::string, index_type> indices_;
				std::function<index_range_type(const char*)> index_range_from_string_;
				index_range_type index_range_all_;
				index_range_type index_range_all_others_;
			};			
		}		
	}
}
