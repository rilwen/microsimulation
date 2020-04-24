#include "contexts.hpp"
#include "history_factory.hpp"
#include "history_factory_registry.hpp"
#include "immutable_context.hpp"
#include "mutable_context.hpp"
#include "person_data.hpp"
#include "population_loader.hpp"
#include "core/csv_file_reader.hpp"
#include "core/stl_utils.hpp"
#include "microsim-core/person_attributes.hpp"
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <cassert>
#include <unordered_set>
#include <utility>
#include <iostream>

namespace averisera {
	namespace microsim {

		PopulationLoader::PopulationLoader(CSV::Delimiter delim, CSV::QuoteCharacter quot_char)
			: _delim(delim), _quot_char(quot_char) {
		}

		static const std::string& EMPTY() {
			static const std::string _empty;
			return _empty;
		}


		void PopulationLoader::load_variables(const std::string& filename, PopulationLoader::value_factory_type_map_t& value_type_map) const {
			CSVFileReader reader(filename, _delim, _quot_char, _csv_has_names);
			if (reader.count_columns() < 2) {
				throw std::runtime_error(boost::str(boost::format("PopulationLoader: at least two columns required in file %s") % filename));
			}
			static const std::string NAME("NAME");
			static const std::string HISTORY_FACTORY("HISTORY_FACTORY");
			const CSVFileReader::index_map_type name_map(reader.read_column_names_map());
			CSVFileReader::check_column_present("PopulationLoader::register_variables", name_map, NAME, filename);
			CSVFileReader::check_column_present("PopulationLoader::register_variables", name_map, HISTORY_FACTORY, filename);
			CSVFileReader::value_map_type value_map;
			CSVFileReader::index_type line_no = 0;
			while (reader.has_next_data_row()) {
				++line_no; // 0 is for header row
				const CSVFileReader::index_type n_read = reader.read_data_row(name_map, value_map);
				if (n_read) {
					if (n_read < 2) {
						throw std::runtime_error(boost::str(boost::format("PopulationLoader::register_variables: incomplete row in file %s, line %d") % filename % line_no));
					}
					const std::string& variable_name = value_map[NAME];
					if (variable_name.empty()) {
						throw std::runtime_error(boost::str(boost::format("PopulationLoader::register_variables: empty variable name in file %s, line %d") % filename % line_no));
					}
					value_type_map[variable_name] = value_map[HISTORY_FACTORY];
				}
			}
		}		

        // Read the history into actor_data if it exists
		static void read_history(const std::unordered_map<std::string, std::string> &value_map, const std::string& var, ActorData& actor_data, const std::string & filename, const CSVFileReader::index_type line_no, const std::string& factory_type_str)
		{
            HistoryData hd(factory_type_str, var);
			const std::string& history_str = Utils::from_string_map<std::string>(value_map, var, EMPTY(), false);
			if (!history_str.empty()) {
                try {
                    hd.append(history_str); // guaranteees that dates are sorted or exception is thrown
                } catch (std::exception& e) {
                    throw std::runtime_error(boost::str(boost::format("PopulationLoader: unable to parse history data for variable %s in file %s, line %d: %s") % var % filename % line_no % e.what()));
                }                
			}
            actor_data.histories.insert(std::make_pair(var, std::move(hd)));
		}

		static const std::string UNLINKED_CHILDBIRTHS("UNLINKED_CHILDBIRTHS"); /* list of additional childbirths which do not result in a
																			   mother-child link in the population. E.g. if we only want
																			   to model the mother and do not provide an entry for the child.
																			   Parsed as a (Date,uint32_t) TimeSeries. */

		static void load_unlinked_childbirths(const CSVFileReader::value_map_type& value_map, PersonData& person) {
			const auto unlk_chldb_iter = value_map.find(UNLINKED_CHILDBIRTHS);
			typedef uint32_t chldbrth_multipl_t;
			typedef TimeSeries<Date, chldbrth_multipl_t> add_chldbrths_ts_t;
			if (unlk_chldb_iter != value_map.end() && !unlk_chldb_iter->second.empty()) {
				const add_chldbrths_ts_t additional_childbirths(add_chldbrths_ts_t::from_string(unlk_chldb_iter->second));
				for (const auto& cb : additional_childbirths) {
					for (chldbrth_multipl_t i = 0; i < cb.second; ++i) {
                        person.childbirths.push_back(cb.first);
					}
				}
			}
		}

		std::vector<PersonData> PopulationLoader::load_persons(const std::string& filename, MutableContext& ctx, const value_factory_type_map_t& value_type_map, bool keep_ids) const {
			CSVFileReader reader(filename, _delim, _quot_char, _csv_has_names);
			static const std::string ID("ID");
			static const std::string SEX("SEX");
			static const std::string ETHNICITY("ETHNICITY");
			static const std::string MOTHER_ID("MOTHER_ID");
			static const std::string DATE_OF_BIRTH("DATE_OF_BIRTH");
			static const std::string CONCEPTION_DATE("CONCEPTION_DATE");
			static const std::string DATE_OF_DEATH("DATE_OF_DEATH");
			
			const CSVFileReader::index_map_type name_map(reader.read_column_names_map());
			static const std::string func_name("PopulationLoader::load_persons");
			CSVFileReader::check_column_present(func_name, name_map, SEX, filename);
			CSVFileReader::check_column_present(func_name, name_map, DATE_OF_BIRTH, filename);
			CSVFileReader::value_map_type value_map;
			std::vector<PersonData> persons;
			persons.reserve(reader.count_data_rows());
			
            std::unordered_set<Actor::id_t> valid_ids;
            std::unordered_set<Actor::id_t> valid_mother_ids;
			CSVFileReader::index_type line_no = 0;
			while (reader.has_next_data_row()) {
				++line_no; // 0 is for the header
				const CSVFileReader::index_type n_read = reader.read_data_row(name_map, value_map);
				if (n_read) {
					CSVFileReader::check_value_present(func_name, value_map, SEX, filename, false);
					const Sex sex = sex_from_string(value_map[SEX]);
					CSVFileReader::check_value_present(func_name, value_map, DATE_OF_BIRTH, filename, false);
					const Date date_of_birth = Utils::from_string<Date>(value_map[DATE_OF_BIRTH]);
					const Actor::id_t id = Utils::from_string_map<Actor::id_t>(value_map, ID, Actor::INVALID_ID, false);
                    if (id != Actor::INVALID_ID) {
                        if (StlUtils::contains(valid_ids, id)) {
                            throw std::runtime_error(boost::str(boost::format("PopulationLoader::load_persons: duplicate ID value %d in file %s, line %d") % id % filename % line_no));
                        } else {
                            valid_ids.insert(id);
                        }
                    }
					const PersonAttributes::ethnicity_t ethnicity = MathUtils::safe_cast<PersonAttributes::ethnicity_t>(Utils::from_string_map<unsigned int>(value_map, ETHNICITY, 0u, false));
                    PersonData person;
                    person.id = id;
                    person.attributes = PersonAttributes(sex, ethnicity);
                    person.date_of_birth = date_of_birth;

					// create and initialize histories
					for (const auto& var : value_type_map) {
						read_history(value_map, var.first, person, filename, line_no, var.second);
					}

					load_unlinked_childbirths(value_map, person);

					// save mother data if available
                    person.mother_id = Utils::from_string_map<Actor::id_t>(value_map, MOTHER_ID, Actor::INVALID_ID, false);
					person.conception_date = Utils::from_string_map<Date>(value_map, CONCEPTION_DATE, Date(), false);
					if (person.mother_id != Actor::INVALID_ID || (!person.conception_date.is_not_a_date())) {
						if (person.mother_id == Actor::INVALID_ID || person.conception_date.is_not_a_date()) {
							throw std::runtime_error(boost::str(boost::format("PopulationLoader::load_persons: both or none %s and %s values required in file %s, line %d") % MOTHER_ID % CONCEPTION_DATE % filename % line_no));
                        } else {
                            valid_mother_ids.insert(person.mother_id);
                        }
					}                    

					// maybe they're dead?
					person.date_of_death = Utils::from_string_map<Date>(value_map, DATE_OF_DEATH, Date(), false);

                    persons.push_back(std::move(person));
				}
			}
			persons.shrink_to_fit();   
            ActorData::sort_by_id(persons);
			for (Actor::id_t mother_id : valid_mother_ids) {
                if (!StlUtils::contains(valid_ids, mother_id)) {                    
                    throw std::runtime_error(boost::str(boost::format("PopulationLoader::load_persons: mother ID %d does not refer to any person, file %s") % mother_id % filename));
                }
            }
            // link mothers back with their children
            for (const PersonData& pd : persons) {
                if (pd.mother_id != Actor::INVALID_ID) {
                    const auto mother_iter = std::lower_bound(persons.begin(), persons.end(), pd.mother_id, [](const PersonData& data, const Actor::id_t& id) { return data.id < id; });
                    assert(mother_iter != persons.end());
                    assert(mother_iter->id == pd.mother_id);
                    mother_iter->children.push_back(pd.id);
                }
            }
            if (keep_ids) {
                ActorData::cleanup_original_ids(persons, ctx);
            } else {
                PersonData::reset_ids(persons, ctx);
            }
			return persons;
		}

        
	}
}
