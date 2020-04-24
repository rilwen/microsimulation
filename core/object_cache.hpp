/*
* (C) Averisera Ltd 2019
*/
#pragma once
#include <boost/format.hpp>
#include <memory>
#include <unordered_map>

namespace averisera {
	/** Very simple object cache. Not thread safe.
	@tparam O Stored object type.
	@tparam T Tag type used to label objects.
	*/
	template <class O, class T = std::string> class ObjectCache {
	public:
		typedef T tag_t;
		typedef O object_t;
		typedef std::shared_ptr<object_t> value_t;

		/** Is the cache empty. */
		bool empty() const {
			return objects_.empty();
		}

		/** Return size (including unused objects). */
		size_t size() const {
			return objects_.size();
		}

		/** Retrieve an object with given tag from the cache, if it exists.
		@throw std::invalid_argument If object with this tag is not present.
		*/
		std::shared_ptr<O> retrieve(const T& tag) const {
			const auto iter = objects_.find(tag);
			if (iter == objects_.end()) {
				throw std::invalid_argument((boost::format("Tag %s not found") % tag).str());
			} else {
				return iter->second;
			}
		}

		/** Store an object in the cache under given tag. */
		void store(const T& tag, const std::shared_ptr<O>& object) {
			objects_.insert(std::make_pair(tag, object));
		}

		/** Does the cache contain the object with given tag? */
		bool contains(const T& tag) const {
			return objects_.count(tag) != 0;
		}

		/** Count number of objects in the cache which are used. */
		size_t number_used() const {
			size_t count = 0;
			for (const auto& tag_object : objects_) {
				count += !tag_object.second.unique();
			}
			return count;
		}

		/** Remove unused objects. 
		@return Number of removed objects.
		*/
		size_t sweep() {
			std::vector<T> unused_tags;
			for (const auto& tag_object : objects_) {
				if (tag_object.second.unique()) {
					unused_tags.push_back(tag_object.first);
				}
			}
			for (const T& tag : unused_tags) {
				// Not thread-safe.
				erase_object(tag);
			}
			return unused_tags.size();
		}
	protected:
		virtual void erase_object(const T& tag) {			
			objects_.erase(tag);
		}
	private:
		std::unordered_map<tag_t, value_t> objects_;
	};
}