// DiceThroneSimulator.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include <iostream>
#include <algorithm>
#include <future>
#include <chrono>
#include <regex>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <filesystem> // c++17

#ifdef _WIN32
#include "sqlite3.h"
#endif

#include "FasterRandomGenerator.h"
#include "Simulator.h"
#include "Simulator4.h"
#include "Zipper.h"
#include "InputParser.h"

#ifdef _WIN32
#include "Server.h"
#endif


void do_precalc(std::string anatomy, std::string ability)
{
    DiceRoller dr{ 10 };
    bool calc_sim = true;
    bool calc_dta = false;
    bool calc_sim4 = true;
    if (calc_sim)
    {
        Simulator sim(dr);
        std::cout << "start " << ability << " " << anatomy << "##################################" << std::endl;
        sim.precalc_ability(ability, anatomy, calc_dta);
        std::cout << ability << " " << anatomy << "ENDED ##################################" << std::endl;
    }
    if (calc_sim4)
    {
        Simulator4 sim4(dr);
        std::cout << "start sim4 " << ability << " " << anatomy << "##################################" << std::endl;
        sim4.precalc_ability(ability, anatomy);
        std::cout << ability << " " << anatomy << "ENDED 4 ##################################" << std::endl;
    }

    
}

int precalc_abilitys(std::vector<std::pair<std::string, std::string>> all_data)
{
    size_t size = all_data.size();
    size_t workers = 0;
    size_t max_workers = std::thread::hardware_concurrency();
    if (max_workers == 0)
    {
        max_workers = 1;
    }
    std::vector<std::future<void>> handles;
    std::chrono::milliseconds span(1000);
    for (auto paire : all_data)
    {
#ifdef _WIN32
        if (workers < max_workers)
        {
            handles.push_back(std::async(std::launch::async, do_precalc, paire.first, paire.second));
            workers++;
        }
        else
        {
            bool found = false;
            while (!found)
            {
                for (size_t i = 0; i < handles.size(); i++)
                {
                    if (handles[i].wait_for(span) != std::future_status::timeout)
                    {
                        handles[i].get();
                        handles[i] = std::async(std::launch::async, do_precalc, paire.first, paire.second);
                        found = true;
                        break;
                    }
                }
            }
        }
#else
        do_precalc(paire.first, paire.second);
#endif
        // loop end
    }
    for (auto& hand : handles)
    {
        hand.get();
    }
    return 0;
}

std::vector<std::pair< std::string, std::string>> get_precalc_vector()
{
    std::vector<std::string> AAABBC = { "Barbarian","Artificer","Cursed Pirate","Samurai","Tactician","Vampire Lord","Gunslinger","Moon Elf","Ninja","Treant","Captain Marvel","Black Panther","Thor","Spider-Man" };
    std::vector<std::string> AABBBC = { "Black Widow" };
    std::vector<std::string> AAABCD = { "Seraph","Pyromancer", "Scarlet Witch" };
    std::vector<std::string> AABBCD = { "Shadow Thief","Huntress","Paladin","Dr Strange","Loki" };
    std::vector<std::string> AABCCD = { "Monk" };

    std::string diceanatomy = "AAABBC";
    std::vector<std::pair< std::string, std::string>> all_data;

    all_data.push_back({ diceanatomy, "SMALL" });
    all_data.push_back({ diceanatomy, "BIG" });

    for (const auto& hero : AAABBC)
    {
        std::vector<std::string> abilities_temp{};
        Helpers::getHeroData(hero, diceanatomy, abilities_temp);
        for (const auto& ab : abilities_temp)
        {
            bool found = false;
            for (const auto& ab2 : all_data)
            {
                if (ab2.second == ab && ab2.first == diceanatomy)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
#ifdef _WIN32
                all_data.push_back({ diceanatomy, ab });
#endif
            }
        }
    }
    //precalc_abilitys(diceanatomy, abilities);

    diceanatomy = "AABBBC";
    for (const auto& hero : AABBBC)
    {
        std::vector<std::string> abilities_temp{};
        Helpers::getHeroData(hero, diceanatomy, abilities_temp);
        for (const auto& ab : abilities_temp)
        {
            bool found = false;
            for (const auto& ab2 : all_data)
            {
                if (ab2.second == ab && ab2.first == diceanatomy)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                all_data.push_back({ diceanatomy, ab });
            }
        }
    }
    //precalc_abilitys(diceanatomy, abilities);

    diceanatomy = "AAABCD";
    for (const auto& hero : AAABCD)
    {
        std::vector<std::string> abilities_temp{};
        Helpers::getHeroData(hero, diceanatomy, abilities_temp);
        for (const auto& ab : abilities_temp)
        {
            bool found = false;
            for (const auto& ab2 : all_data)
            {
                if (ab2.second == ab && ab2.first == diceanatomy)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                all_data.push_back({ diceanatomy, ab });
            }
        }
    }

    diceanatomy = "AABBCD";
    for (const auto& hero : AABBCD)
    {
        std::vector<std::string> abilities_temp{};
        Helpers::getHeroData(hero, diceanatomy, abilities_temp);
        for (const auto& ab : abilities_temp)
        {
            bool found = false;
            for (const auto& ab2 : all_data)
            {
                if (ab2.second == ab && ab2.first == diceanatomy)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                all_data.push_back({ diceanatomy, ab });
            }
        }
    }
    //precalc_abilitys(diceanatomy, abilities);

    diceanatomy = "AABCCD";
    for (const auto& hero : AABCCD)
    {
        std::vector<std::string> abilities_temp{};
        Helpers::getHeroData(hero, diceanatomy, abilities_temp);
        for (const auto& ab : abilities_temp)
        {
            bool found = false;
            for (const auto& ab2 : all_data)
            {
                if (ab2.second == ab && ab2.first == diceanatomy)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                all_data.push_back({ diceanatomy, ab });
            }
        }
    }
    return all_data;
}

void precalc_all(bool do_target, std::string target)
{
    std::filesystem::create_directories("./precalcsDTA/");
    std::filesystem::create_directories("./precalcs4/");
    std::filesystem::create_directories("./precalcs/");
    std::vector<std::pair< std::string, std::string>> all_data = get_precalc_vector();
    precalc_abilitys(all_data);
}

void run_server(DiceRoller& helper)
{
#ifdef _WIN32
    DTServer server{ 80, helper};
#endif
}

bool does_file_exists(const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

void sqliteWriteTxtToDB(sqlite3* db, std::string folder_name, std::string ability_name, std::string diceanatomy)
{
    std::string filename = "./"+folder_name+"/" + ability_name + "-" + diceanatomy + ".txt";
    std::string table_name = ability_name + "_" + diceanatomy;
    std::string replacement = ability_name + "-" + diceanatomy+" ";
    if (diceanatomy == "")
    {
        filename = "./" + folder_name + "/" + ability_name + ".txt";
        table_name = ability_name;
        replacement = ability_name + " ";
    }
    if (!does_file_exists(filename))
    {
        std::cout << filename << " doesnt exist" << std::endl;
        return;
    }
    std::ifstream infile(filename);

    
    //sqlite3* db;
    //sqlite3_open("DiceThroneOdds.db", &db);
    std::string createQuery = "CREATE TABLE IF NOT EXISTS "+ table_name +" (key TEXT PRIMARY KEY, data TEXT);";
    std::cout << createQuery << std::endl;
    sqlite3_stmt* createStmt;
    std::cout << "Creating Table Statement" << std::endl;
    sqlite3_prepare(db, createQuery.c_str(), (int)createQuery.size(), &createStmt, NULL);
    std::cout << "Stepping Table Statement" << std::endl;
    if (sqlite3_step(createStmt) != SQLITE_DONE)
    {
        std::cout << "Didn't Create Table!" << std::endl;
        return ;
    }

    std::string line;
    std::string line2;
    std::string insertQuery = "";
    //std::string insertQuery = "BEGIN TRANSACTION;";
    size_t i = 0;
    size_t j = 0;
    while (std::getline(infile, line))
    {
        std::getline(infile, line2);
        //std::string data = line2;
        std::string data = Zipper::string_compress_encode(line2);
        if (i == 0)
        {
            insertQuery = "INSERT INTO \'" + table_name + "\' SELECT \'" + line + "\' AS key, \'" + data + "\' AS data";
        }
        else
        {
            std::string additional_data = " UNION ALL SELECT \'" + line + "\',\'" + data + "\'";
            insertQuery += additional_data;
        }
        i++;
        if (i >= 498)
        {
            std::cout << j << "\r";
            insertQuery += ";";
            sqlite3_stmt* insertStmt;
            if (sqlite3_prepare_v2(db, insertQuery.c_str(), -1, &insertStmt, NULL) == SQLITE_OK)
            {
                if (sqlite3_step(insertStmt) != SQLITE_DONE)
                {
                    std::cout << "Didn't Insert Item!" << std::endl;
                }
            }
            else
            {
                printf("%s: %s\n", sqlite3_errstr(sqlite3_extended_errcode(db)), sqlite3_errmsg(db));
            }
            
            sqlite3_finalize(insertStmt);
            insertQuery = "";
            i = 0;
        }
        j++;
    }
    if (insertQuery!= "")
    {
        std::cout << "last query lenght "<< insertQuery.size() << std::endl;
        insertQuery += ";";
        sqlite3_stmt* insertStmt;
        sqlite3_prepare(db, insertQuery.c_str(), (int)insertQuery.size(), &insertStmt, NULL);
        
        if (sqlite3_step(insertStmt) != SQLITE_DONE)
        {
            std::cout << "Didn't Insert Item!" << std::endl;
        }
        sqlite3_finalize(insertStmt);
        insertQuery = "";
    }
    
    infile.close();
    

    /**/

}

int loadOrSaveDb(sqlite3* pInMemory, const char* zFilename, int isSave)
{
    int rc;                   /* Function return code */
    sqlite3* pFile;           /* Database connection opened on zFilename */
    sqlite3_backup* pBackup;  /* Backup object used to copy data */
    sqlite3* pTo;             /* Database to copy to (pFile or pInMemory) */
    sqlite3* pFrom;           /* Database to copy from (pFile or pInMemory) */

    rc = sqlite3_open(zFilename, &pFile);
    if (rc == SQLITE_OK)
    {

        pFrom = (isSave ? pInMemory : pFile);
        pTo = (isSave ? pFile : pInMemory);

        pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
        if (pBackup) {
            (void)sqlite3_backup_step(pBackup, -1);
            (void)sqlite3_backup_finish(pBackup);
        }
        rc = sqlite3_errcode(pTo);
    }

    (void)sqlite3_close(pFile);
    return rc;
}

void writeTxtToDB(std::string folder, std::string db_name)
{
    sqlite3* db;
    sqlite3_open(":memory:", &db);
    std::vector<std::pair< std::string, std::string>> all_data = get_precalc_vector();
    for (const auto& data : all_data)
    {
        if (data.second == "SMALL" || data.second == "BIG")
        {
            sqliteWriteTxtToDB(db, folder, data.second, "");
        }
        else
        {
            sqliteWriteTxtToDB(db, folder, data.second, data.first);
        }
        
    }
    std::cout << "write Database" << std::endl;
    loadOrSaveDb(db, db_name.c_str(), 1);
    std::cout << "Database finished!" << std::endl;
    sqlite3_close(db);
}

void writeTxtToDB()
{
    writeTxtToDB("precalcs", "DiceThroneOdds.db");
    writeTxtToDB("precalcsDTA", "DiceThroneOddsDTA.db");
    writeTxtToDB("precalcs4", "DiceThroneOdds4.db");
}

void commandLineCalculation(const InputParser & parser)
{
    std::string hero_name = "";
    size_t sixit = 0;
    size_t tipit = 0;
    size_t samesis = 0;
    size_t wild = 0;
    size_t twiceWild = 0;
    size_t slightlyWild = 0;
    size_t probabilityManipulation = 0;
    size_t cheer = 0;
    size_t cp = 0;
    size_t anzcards = 0;
    size_t rollattempts = 3;
    size_t rerolls = 0;
    bool last_die_is_scarlett = false;
    bool is_default_sim = false;
    bool is_chase = false;
    std::string chase_ability = "";
    std::string dice_anatomy = "";
    std::vector<std::string> abilities = {};
    std::vector<DiceIdx> dice{ 0,0,0,0,0 };

    if (parser.cmdOptionExists("-hero"))
    {
        hero_name = parser.getCmdOption("-hero");
    }

    if (parser.cmdOptionExists("-dice"))
    {
        
        const std::string & dice_s = parser.getCmdOption("-dice");
        size_t ndice = dice_s.size();
        size_t max_input =  ndice;
        if (max_input > 5)
        {
            max_input = 5;
        }
        for (size_t i = 0; i < max_input; i++)
        {
            dice[i] = ((DiceIdx)(dice_s[i] - '0'))-1;
        }
    }
    else
    {
        std::cout << "no dice" << std::endl;
    }

    if (parser.cmdOptionExists("-rolls"))
    {
        const std::string& out_s = parser.getCmdOption("-rolls");
        rollattempts = std::stoi(out_s);
    }

    if (parser.cmdOptionExists("-rerolls"))
    {
        const std::string& out_s = parser.getCmdOption("-rerolls");
        rerolls = std::stoi(out_s);
    }

    if (parser.cmdOptionExists("-anatomy"))
    {
        dice_anatomy = parser.getCmdOption("-anatomy");
    }
    if (parser.cmdOptionExists("-abilities"))
    {
        const std::string& dice_s = parser.getCmdOption("-abilities");
        abilities = Helpers::my_string_split(dice_s, ";");
    }
    if (parser.cmdOptionExists("-chase"))
    {
        chase_ability = parser.getCmdOption("-chase");
        if (chase_ability != "")
        {
            is_chase = true;
        }
    }
    if (parser.cmdOptionExists("-default"))
    {
        const std::string& out_s = parser.getCmdOption("-default");
        if (out_s == "true")
        {
            is_default_sim = true;
        }
        else
        {
            is_default_sim = false;
        }
    }

    if (parser.cmdOptionExists("-scarlett"))
    {
        const std::string& out_s = parser.getCmdOption("-scarlett");
        if (out_s == "true")
        {
            last_die_is_scarlett = true;
        }
        else
        {
            last_die_is_scarlett = false;
        }
    }

    if (parser.cmdOptionExists("-sixit"))
    {
        const std::string& out_s = parser.getCmdOption("-sixit");
        sixit = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-samesies"))
    {
        const std::string& out_s = parser.getCmdOption("-samesies");
        samesis = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-tipit"))
    {
        const std::string& out_s = parser.getCmdOption("-tipit");
        tipit = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-wild"))
    {
        const std::string& out_s = parser.getCmdOption("-wild");
        wild = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-2wild"))
    {
        const std::string& out_s = parser.getCmdOption("-2wild");
        twiceWild = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-swild"))
    {
        const std::string& out_s = parser.getCmdOption("-swild");
        slightlyWild = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-cheer"))
    {
        const std::string& out_s = parser.getCmdOption("-cheer");
        slightlyWild = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-probabilitymanipulation"))
    {
        const std::string& out_s = parser.getCmdOption("-probabilitymanipulation");
        slightlyWild = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-cp"))
    {
        const std::string& out_s = parser.getCmdOption("-cp");
        cp = std::stoi(out_s);
    }
    if (parser.cmdOptionExists("-cards"))
    {
        const std::string& out_s = parser.getCmdOption("-cards");
        anzcards = std::stoi(out_s);
    }

    std::cout << "calculating:" << std::endl;
    std::cout << hero_name << " " << dice_anatomy << " roll atmps "<< rollattempts<< " rerolls "<< rerolls << std::endl;
    std::cout << "dice";
    for (const auto& d : dice)
    {
        std::cout << " " << std::to_string(d);
    }
    if (last_die_is_scarlett)
    {
        std::cout << " last die is scarlett die";
    }
    std::cout << std::endl;
    std::cout << "abilities";
    for (const auto& ab : abilities)
    {
        std::cout << " " << ab;
    }
    std::cout << std::endl;
    std::cout << "default sim " << is_default_sim << " chased ability " << is_chase << " " << chase_ability<<std::endl;
    std::cout << "cards: sixit " << sixit << " samesies " << samesis << " tipit " << tipit << " wild " << wild << " twice as wild " << twiceWild << " slightly wild " << slightlyWild<< " cheer "<< cheer << " probability manipulation "<< probabilityManipulation << std::endl;
    std::cout << "cp " << cp << " number cards " << anzcards << std::endl;

    CardData cardData{};
    cardData.cp = cp;
    cardData.use_max_cards = anzcards;
    cardData.lvlsixit = sixit;
    cardData.lvlsamesis = samesis;
    cardData.lvltip_it = tipit;
    cardData.lvlwild = wild;
    cardData.lvltwiceWild = twiceWild;
    cardData.lvlslightlyWild = slightlyWild;
    cardData.numberProbabilityManipulation = probabilityManipulation;
    cardData.hasCheer = cheer;

    std::vector<OddsResult> ergs;
    DiceRoller dr{ 10 };
    if (last_die_is_scarlett)
    {
        Simulator4 simulator4_{ dr };
        std::cout << "do sim" << std::endl;
        ergs = simulator4_.get_probability(hero_name, abilities, dice_anatomy, is_default_sim, is_chase, chase_ability, dice, cardData, rollattempts, rerolls);
    }
    else
    {
        Simulator simulator_{ dr };
        std::cout << "do sim" << std::endl;
        ergs = simulator_.get_probability(hero_name, abilities, dice_anatomy, is_default_sim, is_chase, chase_ability, dice, cardData, rollattempts, rerolls);
    }
    std::cout << "RESULTS" << std::endl;
    for (const auto& erg : ergs)
    {
        std::cout << erg.ability << " " << erg.get_odds() << " " << erg.get_reroll() << std::endl;
    }
}

void do_precalc_test()
{
    std::string anatomy = "AAABBC";
    std::string ability = "CCCCC";
    DiceRoller dr{ 10 };
    bool calc_sim = true;
    bool calc_dta = true;
    bool calc_sim4 = false;
    if (calc_sim)
    {
        Simulator sim(dr);
        std::cout << "start " << ability << " " << anatomy << "##################################" << std::endl;
        sim.precalc_ability(ability, anatomy, calc_dta);
    }
    if (calc_sim4)
    {
        Simulator4 sim4(dr);
        std::cout << "start sim4 " << ability << " " << anatomy << "##################################" << std::endl;
        sim4.precalc_ability(ability, anatomy);
    }

}

int main(int argc, char* argv[])
{
    // TEST ####
    //do_precalc_test();
    //return 0;
    // TEST ####

    if (argc > 1)
    {
        InputParser parser(argc, argv);
        commandLineCalculation(parser);
        return 0;
    }
    bool docalcs = true;
    bool do_tests = false;
    if (docalcs)
    {
        precalc_all(false, "");
        writeTxtToDB();

        // TESTING ##### 
        //std::filesystem::create_directories("./precalcs4/");
        //do_precalc("AAABBC", "SMALL"); //test
        //writeTxtToDB("precalcs4", "DiceThroneOdds4.db");
    }
    DiceRoller droller{ 5000ULL };
    if (do_tests)
    {
        
        std::cout << "testing stuff" << std::endl;

        Simulator sim(droller);
        Simulator4 sim4(droller);
        //sim.diceRoller_ = &(helper.diceRoller_);
        //sim.test_odds_calc();
        //sim.test_odds_calc_chase();
        sim.combo_test();
        std::cout << "combo test 4 ######################################" << std::endl;
        //sim4.combo_test();
        sim4.test_odds_calc();
        //sim4.test_odds_calc_chase();
        //sim4.combo_test();
        //sim.get_default_probability("Barbarian", false, false, false, false, false, 0, 0);
    }
    run_server(droller);
    std::cout << "server stopped\n";
    return 0;
}