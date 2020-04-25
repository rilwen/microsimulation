#ifndef __AVERISERA_MICROSIM_POPULATION_LOADER_HPP
#define __AVERISERA_MICROSIM_POPULATION_LOADER_HPP

#include "history_data.hpp"
#include "core/csv.hpp"
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace averisera {
	namespace microsim {
		class Contexts;
		class ImmutableContext;
		struct PersonData;

		/** Loads Population member data from CSV files.

        CSV files with Actor-derived objects do not have to contain IDs, but if they do, they must follow the requirements defined in Actor class.
		*/
		class PopulationLoader {
		public:
			/**
			@param delim CSV file delimiter
			@param quot_char CSV quote character
			*/
			PopulationLoader(CSV::Delimiter delim, CSV::QuoteCharacter quot_char);

            typedef std::unordered_map<std::string, std::string> value_factory_type_map_t; /**< Maps variable names to HistoryFactory type strings */

			/** Open a CSV file and load names and types of variables defined there. Skip empty rows.
			Variable name is in column "NAME". History factory is in column "HISTORY_FACTORY".
			*/
			void load_variables(const std::string& filename, value_factory_type_map_t& value_type_map) const;

			/** Load PersonData objects from CSV file. Skip empty rows.
			Searches the file for histories of variables registed in the Context (thus it is not a good idea to name your variable "SEX" or "ETHNICITY").
            @param keep_ids If true, keep original IDs or (if missing) set them to Actor::INVALID_ID. If false, reset the IDs.
            @param value_type_map Map variable name -> value type constant created by register_person_variables.
			@return Vector of PersonData sorted by ID.
			*/
			std::vector<PersonData> load_persons(const std::string& filename, MutableContext& ctx, const value_factory_type_map_t& value_type_map, bool keep_ids) const;

            ///** Check if every data object has a valid id.
            //@tparam AD ActorData or derived struct. */
            //template <class AD> static bool has_valid_ids(const std::vector<AD>& data);
		private:
			static const bool _csv_has_names = true;
			CSV::Delimiter _delim;
			CSV::QuoteCharacter _quot_char;
		};

        /*template <class AD> static bool PopulationLoader::has_valid_ids(const std::vector<AD>& data) {
            for (const AD& ad : data) {
                if (ad.id == Actor::INVALID_ID) {
                    return false;
                }
            }
            return true;
        }*/

        
	}
}

#endif // __AVERISERA_MICROSIM_POPULATION_LOADER_HPP
