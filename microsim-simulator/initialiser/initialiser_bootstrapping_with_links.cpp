#include "../contexts.hpp"
#include "initialiser_bootstrapping_with_links.hpp"
#include "../mutable_context.hpp"
#include "../person.hpp"
#include "core/rng.hpp"
#include "core/stl_utils.hpp"
#include <cassert>
#include <cmath>
#include <unordered_map>
#include <stdexcept>

namespace averisera {
    namespace microsim {
        InitialiserBootstrappingWithLinks::InitialiserBootstrappingWithLinks(std::unique_ptr<PersonDataSampler>&& person_data_sampler, std::vector<std::unique_ptr<const DataPerturbation<PersonData>>>&& person_perturbations, depth_t recursion_limit)
            : InitialiserBootstrapping(std::move(person_data_sampler), std::move(person_perturbations)), _recursion_limit(std::max(0, recursion_limit)) {
        }

        void InitialiserBootstrappingWithLinks::sample(std::vector<PersonData>& added_persons, const pop_size_t remaining_size, const Contexts& ctx) const {
            assert(remaining_size > 0);
            assert(added_persons.empty());
            const size_t idx = static_cast<size_t>(ctx.mutable_ctx().rng().next_uniform(sample_size() - 1));
            assert(idx < sample_size());
            const PersonData& sampled = sample_person(idx);
            visited_t visited;
			const pop_size_t total_family_size = 1 + walk_link_graph(sampled, nullptr, 0, visited, [](const PersonData& p, const PersonData* from) {});
            visited.clear();
            ActorData::id_map_t copies; // map originals to copied clones
            if (total_family_size <= remaining_size) {
				walk_link_graph(sampled, nullptr, 0, visited, [&added_persons, &ctx, &copies, &visited](const PersonData& p, const PersonData* from) {
                    add_copy(added_persons, p, ctx); 
                    PersonData& copy = added_persons.back();
                    // fix child/mother IDs in copies
                    assert(copy.id != Actor::INVALID_ID);
                    copies[p.id] = copy.id;
                    if (from) {
                        assert(StlUtils::contains(visited, from->id));
                        assert(StlUtils::contains(copies, from->id));
                        Actor::id_t from_copy_id = copies[from->id];
                        const auto from_copy_iter = ActorData::find_by_id(added_persons, from_copy_id);
                        assert(from_copy_iter != added_persons.end());
                        if (from->id == p.mother_id) {
                            assert(from_copy_id != copy.mother_id);
                            from_copy_iter->link_child(copy, p.conception_date);
                        } else if (from->mother_id == p.id) {                                                        
                            assert(copy.id != from_copy_iter->mother_id);
                            copy.link_child(*from_copy_iter, from->conception_date);
                        }
                    }
                });
			} else {
				add_copy(added_persons, sampled, ctx);
			}
        }

		template <class F> InitialiserBootstrappingWithLinks::pop_size_t InitialiserBootstrappingWithLinks::walk_link_graph(const PersonData& person, const PersonData* from, const depth_t depth, visited_t& visited, F action) const {
			pop_size_t accumulated = 0;		
            visited.insert(person.id);
            if (std::abs(depth) <= _recursion_limit) {
			    action(person, from);            		
                //const size_t nc = person.children.size();
				for (ActorData::id_t child: person.children) {
					if (child != Actor::INVALID_ID && !StlUtils::contains(visited, child)) {
						const size_t chld_idx = find_by_id(child); // will throw if not found
						const PersonData& chld_dat = sample_person(chld_idx);
						++accumulated;
						accumulated += walk_link_graph(chld_dat, &person, depth + 1, visited, action);
					}
				}
				if (person.mother_id != Actor::INVALID_ID) {					
					if (!StlUtils::contains(visited, person.mother_id)) {
						const size_t moth_idx = find_by_id(person.mother_id); // will throw if not found
						const PersonData& moth_dat = sample_person(moth_idx);
                        ++accumulated;
                        accumulated += walk_link_graph(moth_dat, &person, depth - 1, visited, action);
					}
				}
			}
			return accumulated;
		}
    }
}
