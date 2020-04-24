/*
(C) Averisera Ltd 2015
*/
#ifndef __AVERISERA_AGENT_H
#define __AVERISERA_AGENT_H

#include "history.hpp"
#include <cstdint>
#include <memory>
#include <vector>

namespace averisera {
	namespace microsim {
		struct ActorData;
        class Contexts;
        class HistoryRegistry;
        class ImmutableContext;

		/*!
		\brief Base object representing agents taking part in the simulation. An Actor is a person, a household, an institution or a business entity.
		An Actor has a vector of History.

		Every actor has an ID number which uniquely identifies them. Valid ID numbers are nonzero.
		*/
		class Actor {
		public:
            typedef uint64_t id_t; /*!< Type of Actor IDs. Use 64bits to make sure we do not run out of space. */
            typedef size_t histidx_t; /*!< Type used to index histories stored by the Actor */

			/* Some template type definitions for derived classes */
			template <class Derived> using shared_ptr = std::shared_ptr<Derived>; /*!< Shared pointer */
			template <class Derived> using weak_ptr = std::weak_ptr<Derived>; /*!< Weak pointer */
			template <class Derived, class... Args> static shared_ptr<Derived> make_shared(Args&&... args) {
				return std::shared_ptr<Derived>(new Derived(std::forward<Args>(args)...));
			}
			template <class Derived> static weak_ptr<Derived> weak_from_shared(const shared_ptr<Derived>& shared) {
				return weak_ptr<Derived>(shared);
			}
			template <class Derived> static bool is_valid(const weak_ptr<Derived>& weak) {
				return !weak.expired();
			}
			template <class Derived> static shared_ptr<Derived> shared_from_weak(const weak_ptr<Derived>& weak) {
				return weak.lock();
			}
            
			static const id_t INVALID_ID = 0u; /*!< Used to mark missing ID */
			static const id_t MIN_ID = INVALID_ID + 1; /*!< Minimum ID value, such that (MIN_ID - 1) < MIN_ID in unsigned arithmetic */

			/*!
			\param[in] id ID number, nonzero.
            \param[in] histories Vector of History implementations
			\throw std::domain_error If id < MIN_ID.
			*/
			Actor(id_t id);

            /*! Set histories.
              \throw std::logic_error If already has non-zero number of histories */
            void set_histories(std::vector<std::unique_ptr<History>>&& histories);

            Actor(const Actor&) = delete;

            Actor& operator=(const Actor&) = delete;

			virtual ~Actor();

			/*!
			\return ID number
			*/
			id_t id() const {
				return _id;
			}

			/*! Number of histories stored */
			histidx_t nbr_histories() const {
				return _histories.size();
			}

            /*! Is idx-th history valid (non-null). */
            bool is_history_valid(histidx_t idx) const;

			/*! Reference to idx-th history 
              \throw std::domain_error If no valid idx-th history.
             */
			History& history(histidx_t idx);

			/*! Const reference to idx-th history 
              \throw std::domain_error If no valid idx-th history.
             */
			const History& history(histidx_t idx) const;

            /*! Get the appropriate history registry for the class, or throw an exception
            \throw std::logic_error This class has no history registry */
            virtual const HistoryRegistry& get_history_registry(const ImmutableContext& im_ctx) const = 0;

            /*! Check if Actor has a valid history for this variable */
            bool has_history(const ImmutableContext& im_ctx, const std::string& variable) const;

            /*! Get the history for this variable or throw std::domain_error if not present */
            History& history(const ImmutableContext& im_ctx, const std::string& variable);

            /*! Get the history for this variable or throw std::domain_error if not present */
            const History& history(const ImmutableContext& im_ctx, const std::string& variable) const;

            /*! Get the index of the variable history or throw an exception if not available */
            histidx_t get_variable_index(const Contexts& ctx, const std::string& variable) const;

			/*! Convert to a pure data object */
			ActorData to_data(const ImmutableContext& im_ctx) const;			

			/*! Find a shared pointer to an Actor-derived object in a vector sorted by IDs. We assume no null pointers.
			\tparam AD Derived from ActorImpl<T>.
			\return Iterator to pointer or objects.end() if not found.
			*/
			template <class AD> static typename std::vector<typename AD::shared_ptr>::const_iterator find_by_id(const std::vector<typename AD::shared_ptr>& objects, id_t id) {
				const auto iter = std::lower_bound(objects.begin(), objects.end(), id, [](const typename AD::shared_ptr& ptr, const Actor::id_t& i) { return ptr->id() < i; });
				if (iter != objects.end() && (*iter)->id() == id) {
					return iter;
				} else {
					return objects.end();
				}
			}
		private:
			id_t _id;
			std::vector<std::unique_ptr<History>> _histories; /*!< vector of histories */

			void validate() const;
		};

		/*! Convenience class to automatically define some typedefs. */
		template <class Derived> class ActorImpl : public Actor {
		public:
			using Actor::Actor;

			typedef Actor::shared_ptr<Derived> shared_ptr;
			typedef Actor::shared_ptr<const Derived> const_shared_ptr;
			typedef Actor::weak_ptr<Derived> weak_ptr;
			typedef Actor::weak_ptr<const Derived> const_weak_ptr;

            template <class... Args> static shared_ptr make_shared(Args&&... args) {
                return Actor::make_shared<Derived, Args...>(std::forward<Args>(args)...);
            }
		};
	}
}

#endif

