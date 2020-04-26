// (C) Averisera Ltd 2014-2020
#pragma once
#include "actor.hpp"
//#include "contexts.hpp"
#include "history.hpp"
#include "history_data.hpp"
#include "mutable_context.hpp"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <boost/format.hpp>

namespace averisera {
    namespace microsim {        
        /** Data for Actor object, freely modifiable. Used when initialising Population and when modelling immigration. */		
        struct ActorData {			
			typedef std::unordered_map<std::string, HistoryData> histories_t;
			typedef Actor::id_t id_t;
			histories_t histories;
			id_t id;

            ActorData();
            ActorData(const ActorData&) = delete;
            ActorData& operator=(const ActorData&) = delete;
            ActorData(ActorData&& other) noexcept;
            ActorData& operator=(ActorData&& other);            
            void swap(ActorData& other) noexcept;

            ActorData clone() const;

            /** Get history from data */
			histories_t::iterator get_history(const std::string& name) {
				return histories.find(name);
			}

            /** Get history from data */
            histories_t::const_iterator get_history(const std::string& name) const {
				return histories.find(name);
			}

			
            /** Sort by id property
            @tparam AD ActorData or derived struct */
            template <class AD> static void sort_by_id(std::vector<AD>& data) {
                std::sort(data.begin(), data.end(), [](const AD& a, const AD& b) { return a.id < b.id;  });
            }

            /**  1. Verify that no original ID has been already generated by ctx.
            2. Set maximum ID in ctx to maximum valid original ID in data.
            3. Generate new IDs for those elements in data which had no original ID.
            @throw std::runtime_error If an original ID has been already issued.
            @tparam AD ActorData or derived struct. */
            template <class AD> static void cleanup_original_ids(std::vector<AD>& data, MutableContext& mctx);

            typedef std::unordered_map<id_t, id_t> id_map_t;
            /** Reset loaded IDs. Insert old_id -> new_id mapping to provided map. Upon return, data is sorted by ID.
            @tparam AD ActorData or derived struct. */
            template <class AD> static void reset_ids(std::vector<AD>& data, MutableContext& mctx, id_map_t& old_to_new/*, id_map_t& new_to_old*/);

            /** Find element by ID. Return iterator equal to data.end() if no element with this id.
            @tparam AD ActorData or derived struct. */
            template <class AD> static typename std::vector<AD>::const_iterator find_by_id(const std::vector<AD>& data, id_t id) {
                const auto iter = std::lower_bound(data.begin(), data.end(), id, [](const AD& ad, const id_t& i) { return ad.id < i; });
                if (iter != data.end() && iter->id == id) {
                    return iter;
                } else {
                    return data.end();
                }
            }

            /** Find element by ID. Return iterator equal to data.end() if no element with this id.
            @tparam AD ActorData or derived struct. */
            template <class AD> static typename std::vector<AD>::iterator find_by_id(std::vector<AD>& data, id_t id) {
                const auto iter = std::lower_bound(data.begin(), data.end(), id, [](const AD& ad, const id_t& i) { return ad.id < i; });
                if (iter != data.end() && iter->id == id) {
                    return iter;
                } else {
                    return data.end();
                }
            }

			/** Print to stream */
			void print(std::ostream& os) const;
        };

        template <class AD> void ActorData::reset_ids(std::vector<AD>& data, MutableContext& mctx, id_map_t& old_to_new/*, id_map_t& new_to_old*/) {
            for (AD& ad : data) {
                const id_t old_id = ad.id;
                const id_t new_id = mctx.gen_id();
                ad.id = new_id;
                if (old_id != Actor::INVALID_ID) {
                    old_to_new[old_id] = new_id;
                    //new_to_old[new_id] = old_id;
                }
            }
        }

        template <class AD> void ActorData::cleanup_original_ids(std::vector<AD>& data, MutableContext& mctx) {
            id_t max_orig_id = Actor::INVALID_ID;
            bool has_invalid_ids = false;
            for (AD& ad : data) {
                if (ad.id != Actor::INVALID_ID) {
                    if (ad.id <= mctx.get_max_id()) {
                        throw std::runtime_error(boost::str(boost::format("ActorData: loaded ID %d has been already used") % ad.id));
                    }
                    if (max_orig_id != Actor::INVALID_ID) {
                        max_orig_id = std::max(max_orig_id, ad.id);
                    } else {
                        max_orig_id = ad.id;
                    }
                } else {
                    has_invalid_ids = true;
                }
            }
            mctx.increase_id(max_orig_id);
            if (has_invalid_ids) {
                for (AD& ad : data) {
                    if (ad.id == Actor::INVALID_ID) {
                        ad.id = mctx.gen_id();
                    }
                }
            }
        }

		inline void swap(ActorData& l, ActorData& r) {
			l.swap(r);
		}

		inline std::ostream& operator<<(std::ostream& os, const ActorData& data) {
			data.print(os);
			return os;
		}
    }
}
