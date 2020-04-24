/*
* (C) Averisera Ltd 2019
*/
#pragma once
#include "object_cache.hpp"

namespace averisera {
	/** Version of ObjectCache which counts how many times an object with a given tag was stored into it.
	@tparam O Stored object type.
	@tparam T Tag type used to label objects.
	*/
	template <class O, class T = std::string> class CountingObjectCache: public ObjectCache<O, T> {
	public:
		CountingObjectCache() 
			{}

		/** Store an object in the cache under given tag. 
		@return Object counter value. Return value of 1 means that it's the first object stored in this cache.
		*/
		size_t store(const T& tag, const std::shared_ptr<O>& object) {
			ObjectCache<O, T>::store(tag, object);
			return ++object_counters_[tag];
		}
	protected:
		void erase_object(const T& tag) override {
			ObjectCache<O, T>::erase_object(tag);
			object_counters_.erase(tag);
		}
	private:
		std::unordered_map<typename ObjectCache<O, T>::tag_t, size_t> object_counters_;
	};
}