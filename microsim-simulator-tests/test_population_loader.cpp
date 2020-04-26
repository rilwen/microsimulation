// (C) Averisera Ltd 2014-2020
#include <gtest/gtest.h>
#include "microsim-simulator/mutable_context.hpp"
#include "microsim-simulator/person_data.hpp"
#include "microsim-simulator/population_loader.hpp"
#include "testing/temporary_file.hpp"
#include "core/stl_utils.hpp"
#include <fstream>

using namespace averisera;
using namespace averisera::microsim;
using namespace averisera::testing;

struct TemporaryVariableFile : public TemporaryFile {
	TemporaryVariableFile(bool add_incomplete = false, bool add_empty_name = false) {
		std::ofstream outf(filename);
		outf << "NAME,HISTORY_FACTORY\n";
		outf << "BMI,double\n";
		outf << "\n";
		outf << "smoking,sparse uint8\n";
		outf << "missing,sparse double\n";
		outf << "pregnancy,uint8\n";
		if (add_incomplete) {
			outf << "broken\n";
		}
		if (add_empty_name) {
			outf << ",double\n";
		}
	}
};

TEST(PopulationLoader, RegisterPersonVariables) {
	PopulationLoader loader(CSV::Delimiter::COMMA, CSV::QuoteCharacter::NONE);
	TemporaryVariableFile tvf;
    PopulationLoader::value_factory_type_map_t vmap;
	loader.load_variables(tvf.filename, vmap);
    ASSERT_EQ(4u, vmap.size()) << vmap;
    ASSERT_EQ("double", vmap["BMI"]);
    ASSERT_EQ("sparse double", vmap["missing"]);
    ASSERT_EQ("sparse uint8", vmap["smoking"]);
    ASSERT_EQ("uint8", vmap["pregnancy"]);
}

TEST(PopulationLoader, RegisterPersonVariablesIncomplete) {
	PopulationLoader loader(CSV::Delimiter::COMMA, CSV::QuoteCharacter::NONE);
	TemporaryVariableFile tvf(true);
    PopulationLoader::value_factory_type_map_t vmap;
	ASSERT_THROW(loader.load_variables(tvf.filename, vmap), std::runtime_error);
}

TEST(PopulationLoader, RegisterPersonVariablesEmptyName) {
	PopulationLoader loader(CSV::Delimiter::COMMA, CSV::QuoteCharacter::NONE);
	TemporaryVariableFile tvf(false, true);
    PopulationLoader::value_factory_type_map_t vmap;
	ASSERT_THROW(loader.load_variables(tvf.filename, vmap), std::runtime_error);
}

struct TemporaryPopulationFileWithIDs : public TemporaryFile {
	TemporaryPopulationFileWithIDs() {
		std::ofstream outf(filename);
		outf << "ID;SEX;ETHNICITY;DATE_OF_BIRTH;BMI;smoking;pregnancy;MOTHER_ID;CONCEPTION_DATE\n";		
		outf << "20;MALE;1;2014-04-16;;;;10;2013-07-14\n";
		outf << "10;FEMALE;1;1991-11-05;D[2006-01-01,26.2|2010-01-01,24.5];I[1991-11-05,0|1995-05-01,1|2010-11-02,0];I[1991-11-05,0|2013-07-14,1|2014-04-16,2|2014-04-17,0]\n";
	}
};

TEST(PopulationLoader, LoadPersonsWithIDs) {	
	PopulationLoader loader1(CSV::Delimiter::COMMA, CSV::QuoteCharacter::NONE);
	PopulationLoader loader2(CSV::Delimiter::SEMICOLON, CSV::QuoteCharacter::SINGLE_QUOTE);
	TemporaryVariableFile tvf;
    PopulationLoader::value_factory_type_map_t vmap;
	loader1.load_variables(tvf.filename, vmap);
    TemporaryPopulationFileWithIDs tmp;
	auto mut_ctx = std::make_shared<MutableContext>();
	const std::vector<PersonData> persons = loader2.load_persons(tmp.filename, *mut_ctx, vmap, true);
	ASSERT_EQ(2u, persons.size());
	for (size_t i = 0; i < persons.size(); ++i) {
		EXPECT_EQ(4u, persons[i].histories.size()) << i << ": " << persons[i].histories;
	}
	ASSERT_EQ(10u, persons[0].id);
	ASSERT_EQ(Sex::FEMALE, persons[0].attributes.sex());
	ASSERT_EQ(1u, persons[0].attributes.ethnicity());
	ASSERT_EQ(Date(1991, 11, 5), persons[0].date_of_birth);
	ASSERT_EQ(1u, persons[0].children.size());
	ASSERT_EQ(persons[1].id, persons[0].children[0]);
    //ASSERT_EQ(persons[1].date_of_birth, persons[0].children[0].first);
	ASSERT_EQ(2u, persons[0].get_history("BMI")->second.size());
	
	ASSERT_EQ(20u, persons[1].id);
	ASSERT_EQ(Sex::MALE, persons[1].attributes.sex());
	ASSERT_EQ(1u, persons[1].attributes.ethnicity());
	ASSERT_EQ(Date(2014,4,16), persons[1].date_of_birth);
	ASSERT_EQ(0, persons[1].children.size());
	ASSERT_EQ(Date(2013, 7, 14), persons[1].conception_date);
	ASSERT_EQ(persons[0].id, persons[1].mother_id);
	ASSERT_EQ(0, persons[1].get_history("BMI")->second.size());
}

TEST(PopulationLoader, LoadPersonsWithIDsNoKeep) {
    PopulationLoader loader1(CSV::Delimiter::COMMA, CSV::QuoteCharacter::NONE);
    PopulationLoader loader2(CSV::Delimiter::SEMICOLON, CSV::QuoteCharacter::SINGLE_QUOTE);
    TemporaryVariableFile tvf;
    PopulationLoader::value_factory_type_map_t vmap;
    loader1.load_variables(tvf.filename, vmap);
    TemporaryPopulationFileWithIDs tmp;
    auto mut_ctx = std::make_shared<MutableContext>();
    const std::vector<PersonData> persons = loader2.load_persons(tmp.filename, *mut_ctx, vmap, false);
    ASSERT_EQ(2u, persons.size());
    for (size_t i = 0; i < persons.size(); ++i) {
        EXPECT_EQ(4u, persons[i].histories.size()) << i;
        ASSERT_EQ(i + 1, persons[i].id) << i;
    }
    ASSERT_EQ(Sex::FEMALE, persons[0].attributes.sex());
    ASSERT_EQ(1u, persons[0].attributes.ethnicity());
    ASSERT_EQ(Date(1991, 11, 5), persons[0].date_of_birth);
    ASSERT_EQ(1u, persons[0].children.size());
    ASSERT_EQ(persons[1].id, persons[0].children[0]);
    //ASSERT_EQ(persons[1].date_of_birth, persons[0].children[0].first);
    ASSERT_EQ(2u, persons[0].get_history("BMI")->second.size());

    ASSERT_EQ(Sex::MALE, persons[1].attributes.sex());
    ASSERT_EQ(1u, persons[1].attributes.ethnicity());
    ASSERT_EQ(Date(2014, 4, 16), persons[1].date_of_birth);
    ASSERT_EQ(0, persons[1].children.size());
    ASSERT_EQ(Date(2013, 7, 14), persons[1].conception_date);
    ASSERT_EQ(persons[0].id, persons[1].mother_id);
    ASSERT_EQ(0, persons[1].get_history("BMI")->second.size());
}

struct TemporaryPopulationFileWithoutIDs : public TemporaryFile {
	TemporaryPopulationFileWithoutIDs() {
		std::ofstream outf(filename);
		outf << "SEX;ETHNICITY;DATE_OF_BIRTH;BMI;smoking;pregnancy;UNLINKED_CHILDBIRTHS\n";
		outf << "FEMALE;1;1991-11-05;D[2006-01-01,26.2|2010-01-01,24.5];I[1991-11-05,0|1995-05-01,1|2010-11-02,0];I[1991-11-05,0|2013-07-14,1|2014-04-16,2|2014-04-17,0];[2014-04-16,1]\n";
		outf << "MALE;1;2014-04-16;;;;\n";
	}
};

TEST(PopulationLoader, LoadPersonsWithoutIDs) {
	PopulationLoader loader1(CSV::Delimiter::COMMA, CSV::QuoteCharacter::NONE);
	PopulationLoader loader2(CSV::Delimiter::SEMICOLON, CSV::QuoteCharacter::SINGLE_QUOTE);
	TemporaryVariableFile tvf;
    PopulationLoader::value_factory_type_map_t vmap;
	loader1.load_variables(tvf.filename, vmap);
	TemporaryPopulationFileWithoutIDs tmp;
	auto mut_ctx = std::make_shared<MutableContext>();
	const auto persons = loader2.load_persons(tmp.filename, *mut_ctx, vmap, true);
	ASSERT_EQ(2u, persons.size());
	for (size_t i = 0; i < persons.size(); ++i) {
		ASSERT_EQ(4u, persons[i].histories.size()) << i;
		ASSERT_EQ(i + 1, persons[i].id) << i;		
        ASSERT_EQ(Actor::INVALID_ID, persons[i].mother_id) << i;
        for (const auto& cd : persons[i].children) {
			ASSERT_EQ(Actor::INVALID_ID, cd) << i;
        }
	}
	ASSERT_EQ(Sex::FEMALE, persons[0].attributes.sex());
	ASSERT_EQ(1u, persons[0].attributes.ethnicity());
	ASSERT_EQ(Date(1991, 11, 5), persons[0].date_of_birth);
    const HistoryData& bmi_hist = (persons[0].get_history("BMI")->second);
	ASSERT_EQ(2u, bmi_hist.size());
	ASSERT_EQ(Date(2006, 1, 1), bmi_hist.dates()[0]);
	ASSERT_EQ(26.2, bmi_hist.values().as<double>()[0]);
	ASSERT_EQ(Date(2010, 1, 1), bmi_hist.dates()[1]);
	ASSERT_EQ(24.5, bmi_hist.values().as<double>()[1]);
    ASSERT_EQ(0u, persons[0].children.size());
	ASSERT_EQ(1u, persons[0].childbirths.size());

	ASSERT_EQ(Sex::MALE, persons[1].attributes.sex());
	ASSERT_EQ(1u, persons[1].attributes.ethnicity());
	ASSERT_EQ(Date(2014, 4, 16), persons[1].date_of_birth);	
	ASSERT_EQ(0, persons[1].get_history("BMI")->second.size());
    ASSERT_EQ(0, persons[1].children.size());
	ASSERT_EQ(0, persons[1].childbirths.size());
}
