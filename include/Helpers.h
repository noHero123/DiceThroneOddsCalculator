#pragma once
#include <algorithm>
#include <fstream>
#include "Types.h"
#include "sqlite3.h"
#include "Zipper.h"

class Helpers
{
public:

    Helpers() 
    {
    }

    void closeDB()
    {
        if(db != nullptr)
        {
            sqlite3_close(db);
        }
        last_db_path = "";
    }

    void closeMatrixDB()
    {
        if (matrixdb != nullptr)
        {
            sqlite3_close(matrixdb);
        }
        last_matrixdb_path = "";
    }

    int choose(int n, int k)
    {
        if (k == 0)
        {
            return 1;
        }
        return (n * choose(n - 1, k - 1)) / k;
    }

    std::string sqlite_get_data(std::string table_name, std::string search, bool isDTA, bool sim4 = false)
    {
        int rc;                   /* Function return code */
        
        std::string path = isDTA ? "DiceThroneOddsDTA.db" : "DiceThroneOdds.db";
        if (sim4)
        {
            path = "DiceThroneOdds4.db";
        }
        if (path != last_db_path || db == nullptr)
        {
            closeDB();
            rc = sqlite3_open(path.c_str(), &db);
            if (rc != SQLITE_OK)
            {
                std::cout << "database " << path << "is missing" << std::endl;
                return "";
            }
            last_db_path = path;
        }
        
        
        std::string selectQuery = "SELECT * FROM " + table_name + " WHERE key == \'" + search + "\';";
        sqlite3_stmt* selectStmt;
        sqlite3_prepare(db, selectQuery.c_str(), (int)selectQuery.size(), &selectStmt, NULL);
        int ret_code = 0;
        if ((ret_code = sqlite3_step(selectStmt)) != SQLITE_ROW)
        {
            std::cout << "cant find "<< table_name << std::endl;
            return "";
        }
        const unsigned char* txt_ptr = sqlite3_column_text(selectStmt, 1);
        if (txt_ptr == nullptr)
        {
            std::cout << "error data null" << std::endl;
            return "";
        }
        std::string txt = std::string(reinterpret_cast<const char*>(txt_ptr));
        sqlite3_finalize(selectStmt);
        std::string txt1 = txt;
        if (txt != "")
        {
            txt1 = Zipper::string_decompress_decode(txt);
        }
        return txt1;
    }

    void sqlite_write_matrix_data(std::string key, std::string uncompressed_data, bool isDTA, bool sim4, size_t thread)
    {
        std::string table_name = "DTMatrix";
        std::string dbname = "Matrix_t" +std::to_string(thread) +".db";
        if (sim4) dbname = "Matrix4_t" + std::to_string(thread) + ".db";
        if (isDTA) dbname = "MatrixDTA_t" + std::to_string(thread) + ".db";
        int rc;                   /* Function return code */

        if (dbname != last_matrixdb_path || matrixdb == nullptr)
        {
            closeMatrixDB();
            rc = sqlite3_open(dbname.c_str(), &matrixdb);
            if (rc != SQLITE_OK)
            {
                std::cout << "database " << dbname << "is missing" << std::endl;
                return;
            }
            last_matrixdb_path = dbname;
            //create matrix table:
            

            std::string createQuery = "CREATE TABLE IF NOT EXISTS " + table_name + " (key TEXT PRIMARY KEY, data TEXT);";
            std::cout << createQuery << std::endl;
            sqlite3_stmt* createStmt;
            std::cout << "Creating Table Statement" << std::endl;
            sqlite3_prepare(matrixdb, createQuery.c_str(), (int)createQuery.size(), &createStmt, NULL);
            std::cout << "Stepping Table Statement" << std::endl;
            if (sqlite3_step(createStmt) != SQLITE_DONE)
            {
                std::cout << "Didn't Create Table!" << std::endl;
                return;
            }
            sqlite3_finalize(createStmt);
        }

        std::string insertQuery = "";
        std::string data = Zipper::string_compress_encode(uncompressed_data);
        insertQuery = "INSERT INTO \'" + table_name + "\' SELECT \'" + key + "\' AS key, \'" + data + "\' AS data;";
        sqlite3_stmt* insertStmt;
        if (sqlite3_prepare_v2(matrixdb, insertQuery.c_str(), -1, &insertStmt, NULL) == SQLITE_OK)
        {
            if (sqlite3_step(insertStmt) != SQLITE_DONE)
            {
                std::cout << "Didn't Insert Item!" << std::endl;
            }
        }
        else
        {
            printf("%s: %s\n", sqlite3_errstr(sqlite3_extended_errcode(matrixdb)), sqlite3_errmsg(matrixdb));
        }
        sqlite3_finalize(insertStmt);
    }

    void sqlite_write_matrix_data_fast(std::string key, std::string uncompressed_data, bool isDTA, bool sim4 = false)
    {
        std::string dbname = ":memory:";
        int rc;                   /* Function return code */

        if (dbname != last_matrixdb_path || matrixdb == nullptr)
        {
            closeMatrixDB();
            rc = sqlite3_open(dbname.c_str(), &matrixdb);
            if (rc != SQLITE_OK)
            {
                std::cout << "database " << dbname << "is missing" << std::endl;
                return;
            }
            last_matrixdb_path = dbname;
            //create matrix table:
            std::string table_name = "DTMatrix";

            std::string createQuery = "CREATE TABLE IF NOT EXISTS " + table_name + " (key TEXT PRIMARY KEY, data TEXT);";
            std::cout << createQuery << std::endl;
            sqlite3_stmt* createStmt;
            std::cout << "Creating Table Statement" << std::endl;
            sqlite3_prepare(matrixdb, createQuery.c_str(), (int)createQuery.size(), &createStmt, NULL);
            std::cout << "Stepping Table Statement" << std::endl;
            if (sqlite3_step(createStmt) != SQLITE_DONE)
            {
                std::cout << "Didn't Create Table!" << std::endl;
                return;
            }
            sqlite3_finalize(createStmt);
        }

        std::string table_name = "DTMatrix";
        std::string insertQuery = "";
        std::string data = Zipper::string_compress_encode(uncompressed_data);
        insertQuery = "INSERT INTO \'" + table_name + "\' SELECT \'" + key + "\' AS key, \'" + data + "\' AS data;";
        sqlite3_stmt* insertStmt;
        if (sqlite3_prepare_v2(matrixdb, insertQuery.c_str(), -1, &insertStmt, NULL) == SQLITE_OK)
        {
            if (sqlite3_step(insertStmt) != SQLITE_DONE)
            {
                std::cout << "Didn't Insert Item!" << std::endl;
            }
        }
        else
        {
            printf("%s: %s\n", sqlite3_errstr(sqlite3_extended_errcode(matrixdb)), sqlite3_errmsg(matrixdb));
        }
        sqlite3_finalize(insertStmt);
    }

    void add_to_save_string(std::string key, std::string uncompressed_data)
    {
        if (matrix_db_save_string == "")
        {
            std::string table_name = "DTMatrix";
            std::string data = Zipper::string_compress_encode(uncompressed_data);
            std::string createQuery = "INSERT INTO \'" + table_name + "\' SELECT \'" + key + "\' AS key, \'" + data + "\' AS data";
            matrix_db_save_string += createQuery;
        }
        else
        {
            std::string data = Zipper::string_compress_encode(uncompressed_data);
            std::string additional_data = " UNION ALL SELECT \'" + key + "\',\'" + data + "\'";
            matrix_db_save_string += additional_data;
        }
        
    }
    
    void write_to_db(bool isDTA, bool sim4 = false)
    {
        if (matrix_db_save_string == "")
        {
            return;
        }
        std::string dbname = "Matrix.db";
        if (sim4) dbname = "Matrix4.db";
        if (isDTA) dbname = "MatrixDTA.db";
        int rc;                   /* Function return code */

        if (dbname != last_matrixdb_path || matrixdb == nullptr)
        {
            closeMatrixDB();
            rc = sqlite3_open(dbname.c_str(), &matrixdb);
            if (rc != SQLITE_OK)
            {
                std::cout << "database " << dbname << "is missing" << std::endl;
                return;
            }
            last_matrixdb_path = dbname;
            //create matrix table:
            std::string table_name = "DTMatrix";

            std::string createQuery = "CREATE TABLE IF NOT EXISTS " + table_name + " (key TEXT PRIMARY KEY, data TEXT);";
            std::cout << createQuery << std::endl;
            sqlite3_stmt* createStmt;
            std::cout << "Creating Table Statement" << std::endl;
            sqlite3_prepare(matrixdb, createQuery.c_str(), (int)createQuery.size(), &createStmt, NULL);
            std::cout << "Stepping Table Statement" << std::endl;
            if (sqlite3_step(createStmt) != SQLITE_DONE)
            {
                std::cout << "Didn't Create Table!" << std::endl;
                return;
            }
            sqlite3_finalize(createStmt);
        }

        std::string table_name = "DTMatrix";
        std::string insertQuery = "";
        insertQuery = matrix_db_save_string+ "; ";
        sqlite3_stmt* insertStmt;
        if (sqlite3_prepare_v2(matrixdb, insertQuery.c_str(), -1, &insertStmt, NULL) == SQLITE_OK)
        {
            if (sqlite3_step(insertStmt) != SQLITE_DONE)
            {
                std::cout << "Didn't Insert Item!" << std::endl;
            }
        }
        else
        {
            printf("%s: %s\n", sqlite3_errstr(sqlite3_extended_errcode(matrixdb)), sqlite3_errmsg(matrixdb));
        }
        sqlite3_finalize(insertStmt);
        matrix_db_save_string = "";
    }

    void sqlite_write_memory_matrix(bool isDTA, bool sim4 = false)
    {
        loadOrSaveDb(matrixdb, isDTA, sim4);
    }

    int loadOrSaveDb(sqlite3* pInMemory,  bool isDTA, bool sim4 = false)
    {
        int isSave = 1;
        std::string dbname = "Matrix.db";
        if (sim4) dbname = "Matrix4.db";
        if (isDTA) dbname = "MatrixDTA.db";
        const char* zFilename = dbname.c_str();
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

    std::string sqlite_get_matrix_data(std::string search, bool isDTA, bool sim4)
    {
        std::string dbname = "Matrix.db";
        if (sim4) dbname = "Matrix4.db";
        if (isDTA) dbname = "MatrixDTA.db";
        return sqlite_get_matrix_data_thread(dbname, search);
    }

    std::string sqlite_get_matrix_data_thread(std::string search, bool isDTA, bool sim4, size_t thread)
    {
        std::string dbname = "Matrix_t" + std::to_string(thread) + ".db";
        if (sim4) dbname = "Matrix4_t" + std::to_string(thread) + ".db";
        if (isDTA) dbname = "MatrixDTA_t" + std::to_string(thread) + ".db";

        return sqlite_get_matrix_data_thread(dbname, search);
    }

    std::string sqlite_get_matrix_data_thread(std::string dbname, std::string search)
    {
        int rc;                   /* Function return code */
        std::string table_name = "DTMatrix";

        if (dbname != last_matrixdb_path || matrixdb == nullptr)
        {
            closeMatrixDB();
            rc = sqlite3_open(dbname.c_str(), &matrixdb);
            if (rc != SQLITE_OK)
            {
                std::cout << "database " << dbname << "is missing" << std::endl;
                return "";
            }
            last_matrixdb_path = dbname;
            //create matrix table:


            std::string createQuery = "CREATE TABLE IF NOT EXISTS " + table_name + " (key TEXT PRIMARY KEY, data TEXT);";
            std::cout << createQuery << std::endl;
            sqlite3_stmt* createStmt;
            std::cout << "Creating Table Statement" << std::endl;
            sqlite3_prepare(matrixdb, createQuery.c_str(), (int)createQuery.size(), &createStmt, NULL);
            std::cout << "Stepping Table Statement" << std::endl;
            if (sqlite3_step(createStmt) != SQLITE_DONE)
            {
                std::cout << "Didn't Create Table!" << std::endl;
                return "";
            }
            sqlite3_finalize(createStmt);
        }


        std::string selectQuery = "SELECT * FROM " + table_name + " WHERE key == \'" + search + "\';";
        sqlite3_stmt* selectStmt;
        sqlite3_prepare(matrixdb, selectQuery.c_str(), (int)selectQuery.size(), &selectStmt, NULL);
        int ret_code = 0;
        if ((ret_code = sqlite3_step(selectStmt)) != SQLITE_ROW)
        {
            std::cout << "cant find "<< search << " in " << table_name << std::endl;
            return "";
        }
        const unsigned char* txt_ptr = sqlite3_column_text(selectStmt, 1);
        if (txt_ptr == nullptr)
        {
            std::cout << "error data null" << std::endl;
            return "";
        }
        std::string txt = std::string(reinterpret_cast<const char*>(txt_ptr));
        sqlite3_finalize(selectStmt);
        std::string txt1 = txt;
        if (txt != "")
        {
            txt1 = Zipper::string_decompress_decode(txt);
        }
        return txt1;
    }

    static size_t get_number_lines(const std::string& name)
    {
        std::ifstream myfile(name);

        // new lines will be skipped unless we stop it from happening:    
        myfile.unsetf(std::ios_base::skipws);

        // count the newlines with an algorithm specialized for counting:
        size_t line_count = std::count(
            std::istream_iterator<char>(myfile),
            std::istream_iterator<char>(),
            '\n');
        size_t lc = line_count / 2;
        std::cout << "file has " << line_count << " lines " << lc << std::endl;
        return lc;
    }

    static bool file_exists(const std::string& name) {
        std::ifstream f(name.c_str());
        return f.good();
    }

    static std::vector<DiceIdx> transformDiceAnatomy(std::string anatomy)
    {
        std::vector<DiceIdx> erg{ 0,0,0,0,0,0 };
        DiceIdx anasize = (DiceIdx)std::min(anatomy.size(), (size_t)6);
        for (DiceIdx i = 0; i < anasize; i++)
        {
            DiceIdx idx = 0;
            if (anatomy[i] == 'B')
            {
                idx = 1;
            }
            if (anatomy[i] == 'C')
            {
                idx = 2;
            }
            if (anatomy[i] == 'D')
            {
                idx = 3;
            }
            if (anatomy[i] == 'E')
            {
                idx = 4;
            }
            if (anatomy[i] == 'F')
            {
                idx = 5;
            }
            erg[i] = idx;
        }
        return erg;
    }

    static std::vector<DiceIdx>  transformAbility(std::string ability)
    {
        std::vector<DiceIdx> erg{ 0,0,0,0,0,0 };
        if (ability == "SMALL")
        {
            return { 7,0,0,0,0,0 };
        }
        if (ability == "BIG")
        {
            return { 8,0,0,0,0,0 };
        }
        for (size_t i = 0; i < ability.size(); i++)
        {
            size_t idx = 0;
            if (ability[i] == 'B')
            {
                idx = 1;
            }
            if (ability[i] == 'C')
            {
                idx = 2;
            }
            if (ability[i] == 'D')
            {
                idx = 3;
            }
            if (ability[i] == 'E')
            {
                idx = 4;
            }
            if (ability[i] == 'F')
            {
                idx = 5;
            }
            erg[idx]++;
        }
        return erg;
    }


    static long long factorial(long long k) {
        long long res = 1;
        while (k > 0) {
            res *= k--;
        }
        return res;
    }

    static void getHeroData(std::string hero_name, std::string& diceanatomy, std::vector<std::string>& abilities)
    {
        diceanatomy = "";
        abilities.clear();
        if (hero_name == "Barbarian")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "AABB", "AACC","AAACC", "CCC", "CCCC", "CCCCC", "BBB", "BBBB", "BBBBB" };
        }
        if (hero_name == "Artificer")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "BBBC", "AABBB", "CCCC", "CCCCC", "AAAC", "AABB", "CCC", "ABBC" };
        }
        if (hero_name == "Cursed Pirate")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA","BBB", "ABBC", "CCCC", "ACCC", "CCCCC" };
        }
        if (hero_name == "Samurai")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "AACC", "AABBB", "CCCC", "CCCCC", "ABC", "CCC", "BBB" };
        }
        if (hero_name == "Tactician")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "AACC", "AAACC", "ABBB", "CCCC", "CCCCC", "AAAB", "ABBC", "BBBB", "CCC" };
        }
        if (hero_name == "Vampire Lord")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "BBB", "AAACC", "BBBC", "CCCC", "CCCCC", "CCC", "AACC", "ABCC", "ABB", "AAC" };
        }
        if (hero_name == "Gunslinger")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "AACC", "AABBB", "CCCC", "CCCCC", "BBC", "BBB", "CCC" };
        }
        if (hero_name == "Moon Elf")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "AAACC", "ACCC","AABBB", "CCCC", "CCCCC", "AAC", "BBB", "ABBC","CCC" };
        }
        if (hero_name == "Ninja")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "BBBB", "ABBC","AAABB", "CCCC", "CCCCC", "BBBCC", "BBB", "AACC","CCC" };
        }
        if (hero_name == "Treant")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "BBCC", "AABBB","AACC", "ABB", "BBB", "CCCC", "CCC","CCCCC" };
        }
        //marvel
        if (hero_name == "Captain Marvel")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "AACC", "AABCC","ABBB", "CCCC", "CCCCC", "BBBC", "BCC", "AABB","CCC" };
        }
        if (hero_name == "Black Panther")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "AACC", "BBB","AABBC", "CCCC", "CCCCC", "CCC" };
        }
        if (hero_name == "Thor")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "ABBC", "AACC","AABBB", "AAACC", "BBB", "CCCC", "CCC", "CCCCC" };
        }
        if (hero_name == "Spider-Man" || hero_name == "Spider Man")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA", "AAAA", "AAAAA", "AABBB", "AACC","ABBC", "CCCC", "CCCCC", "CCC", "BBC" };
        }
        if (hero_name == "Santa")
        {
            diceanatomy = "AAABBC";
            abilities = { "AAA","AAAA","AAAAA","BBBCC", "CCCC", "BBBB", "AABBC", "AABB", "CCC", "CCCCC"};
        }

        if (hero_name == "Seraph")
        {
            diceanatomy = "AAABCD";
            abilities = { "AAA", "AAAA", "AAAAA", "AAAB", "ABCD", "CCD", "DDDD", "DDDDD", "ACC", "ABB", "AAAD", "AAAC", "DDD" };
        }
        if (hero_name == "Pyromancer")
        {
            diceanatomy = "AAABCD";
            abilities = { "AAA", "AAAA", "AAAAA", "CC", "CCC","CCCC", "CCCCC", "ABCD", "AAAD", "DDDD", "DDDDD","AABB", "BBCC", "DDD" };
        }
        if (hero_name == "Scarlet Witch")
        {
            diceanatomy = "AAABCD";
            abilities = { "AAA", "AAAA", "AAAAA", "AABB", "CC","ABCD", "ADDD", "DDDD", "DDD", "BBC", "AAC", "DDDDD"};
        }
        if (hero_name == "Krampus")
        {
            diceanatomy = "AAABCD";
            abilities = { "AAA","AAAA","AAAAA","CCC", "CC", "DDDD", "ABCD", "AABB", "DDDDD","DDD", "AAAC"};
        }

        if (hero_name == "Black Widow")
        {
            diceanatomy = "AABBBC";
            abilities = { "BBB", "BBBB", "BBBBB", "AABC", "AABCC","AAABB", "AABB", "ABBB", "CCCC", "CCC", "CCCCC" };
        }

        if (hero_name == "Shadow Thief")
        {
            diceanatomy = "AABBCD";
            abilities = { "AAA", "AAAA", "AAAAA", "BB", "BBB","BBBB", "BBBBB", "DDD", "CC", "CCC", "CCCC","CCCCC", "AADD", "ABCD", "DDDDD" };
        }
        if (hero_name == "Huntress")
        {
            diceanatomy = "AABBCD";
            abilities = { "AAA", "AAAA", "AAAAA", "BBBD","CC", "AAAC", "DDDD", "AABBC", "DDDDD", "DDD", "ABD", "AAD", "BBD" };
        }
        if (hero_name == "Paladin")
        {
            diceanatomy = "AABBCD";
            abilities = { "CC", "BBBD", "AAABB","AAAD", "DDDD", "DDDDD", "AAD", "AAB", "ABCD", "DDD" };
        }
        if (hero_name == "Dr Strange")
        {
            diceanatomy = "AABBCD";
            abilities = { "BBDD", "BBBB", "ABCD", "AACC", "DDDD","DDDDD" };
        }
        if (hero_name == "Loki")
        {
            diceanatomy = "AABBCD";
            abilities = { "AAA", "AAAA", "AAAAA", "AABB", "CC","ABCD", "DDDD", "DDDDD", "BBC", "BBB", "CCC", "DDD" };
        }

        if (hero_name == "Monk")
        {
            diceanatomy = "AABCCD";
            abilities = { "AAA", "AAAA", "AAAAA", "CCC","AAAB", "BBB", "DDDD", "DDDDD", "DDD", "AACC", "ABCD" };
        }
        abilities.push_back("SMALL");
        abilities.push_back("BIG");
        return;
    }

    static std::vector<std::string> my_string_split(std::string message, std::string delimiter, std::string delimiter2)
    {
        std::vector<std::string> result{};
        size_t pos = 0;
        size_t pos2 = 0;
        std::string s = message;
        std::string token = "";
        pos = s.find(delimiter);
        pos2 = s.find(delimiter2);
        std::string last = "";
        while (pos != std::string::npos || pos2 != std::string::npos) {
            if (pos < pos2)
            {
                token = s.substr(0, pos);
                if (last + token != "")
                {
                    result.push_back(last + token);
                }
                s.erase(0, pos + delimiter.length());
                last = delimiter;
            }
            else
            {
                token = s.substr(0, pos2);
                if (last + token != "")
                {
                    result.push_back(last + token);
                }
                s.erase(0, pos2 + delimiter2.length());
                last = delimiter2;
            }
            pos = s.find(delimiter);
            pos2 = s.find(delimiter2);
        }
        if ((last + s) != "")
        {
            result.push_back(last + s);
        }
        return result;
    }

    static std::vector<std::string> my_string_split(std::string message, std::string delimiter)
    {
        std::vector<std::string> result{};
        size_t pos = 0;
        std::string s = message;
        std::string token = "";
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            result.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        if (s != "")
        {
            result.push_back(s);
        }
        return result;
    }

    static void getCombinations(const std::vector<DiceIdx>& numbers, size_t size, std::vector<DiceIdx>& line, std::vector<std::vector<DiceIdx>>& ergs) {
        for (size_t i = 0; i < numbers.size(); i++) {
            line.push_back(numbers[i]);
            if (size <= 1)
            {
                ergs.push_back(line);
                line.erase(line.end() - 1);
            }
            else {
                getCombinations(numbers, size - 1, line, ergs); // Recursion happens here
                line.erase(line.end() - 1);
            }
        }
    }

    static std::vector<bool> getBitArray(size_t x, size_t number_bits)
    {

        std::vector<bool> arr;
        arr.resize(number_bits);
        for (size_t i = 0; i < number_bits; ++i) {
            arr[i] = ((x >> i) & 1) ? true : false;
        }
        return arr;
    }

    static Card generateCard(std::string name, size_t level)
    {
        size_t lvl = std::max(level, (size_t)1);
        Card c{};
        if (name == "sixit")
        {
            c.name = "sixit";
            c.card_id = 0;
            c.lvl = lvl;
            c.cp_cost = 1;
            c.function_to_call = 0;
            c.dice_manipulateable = 1;
            if (lvl >= 2)
            {
                c.cp_cost = 0;
            }
            if (lvl >= 2)
            {
                c.lvl = 2;
            }
        }
        if (name == "samesis")
        {
            c.name = "samesis";
            c.card_id = 1;
            c.lvl = lvl;
            c.cp_cost = 1;
            c.function_to_call = 1; ///samesis with 1 die
            c.dice_manipulateable = 1;
            if (lvl == 2)
            { //samesis with 1 die
                c.cp_cost = 0;
                c.function_to_call = 1;
            }
            if (lvl >= 3)
            { //samesis with 2 die
                c.cp_cost = 2;
                c.function_to_call = 2;
                c.dice_manipulateable = 2;
            }
            if (lvl >= 3)
            {
                c.lvl = 3;
            }

        }
        if (name == "tipit")
        {
            c.name = "tipit";
            c.card_id = 2;
            c.lvl = lvl;
            c.cp_cost = 1;
            c.function_to_call = 3; ///default: tipit with +-1
            c.dice_manipulateable = 1;
            if (lvl == 2)
            { //tipit +-1
                c.cp_cost = 0;
                c.function_to_call = 3;
            }
            if (lvl >= 3)
            { //tipit +-2
                c.cp_cost = 0;
                c.function_to_call = 4;
            }
            if (lvl >= 3)
            {
                c.lvl = 3;
            }
        }
        if (name == "wild")
        {
            c.name = "wild";
            c.card_id = 3;
            c.lvl = lvl;
            c.cp_cost = 2;
            c.function_to_call = 5; //wild one die
            c.dice_manipulateable = 1;
            if (lvl == 2)
            {
                c.cp_cost = 1;
            }
            if (lvl >= 3)
            {
                c.cp_cost = 0;
            }
            if (lvl >= 3)
            {
                c.lvl = 3;
            }
        }
        if (name == "twice as wild")
        {
            c.name = "tawild";
            c.card_id = 4;
            c.lvl = lvl;
            c.cp_cost = 3;
            c.function_to_call = 6; //wild two die
            c.dice_manipulateable = 2;
            if (lvl == 2)
            {
                c.cp_cost = 2;
            }
            if (lvl == 3)
            {
                c.cp_cost = 1;
            }
            if (lvl >= 4)
            {
                c.cp_cost = 0;
            }
            if (lvl >= 4)
            {
                c.lvl = 4;
            }
        }
        if (name == "slightly wild")
        {
            c.name = "slightwild";
            c.card_id = 5;
            c.lvl = lvl;
            c.cp_cost = 1;
            c.dice_manipulateable = 1;
            c.function_to_call = 7; //not so wild one die
            if (lvl >= 3)
            {
                c.cp_cost = 0;
            }
            if (lvl >= 3)
            {
                c.lvl = 3;
            }
            else
            {
                if (lvl == 2)
                {
                    // on lvl 2 you can just use it on other heros, but not other cp/effect than lvl 1
                    c.lvl = 1;
                }
            }
        }
        if (name == "cheer")
        {
            c.name = "cheer";
            c.card_id = 6;
            c.lvl = 1;
            c.cp_cost = 0;
            c.function_to_call = 0;
            c.dice_manipulateable = 1;
        }
        if (name == "probability manipulation")
        {
            c.name = "probmani";
            c.card_id = 7;
            c.lvl = 1;
            c.cp_cost = 0;
            c.function_to_call = 8;
            c.dice_manipulateable = 1;
        }


        c.can_use = true;
        return c;
    }

    static std::vector<Card> getCards(size_t lvlsixit, size_t lvlsamesis, size_t lvlTip, size_t lvlWild, size_t lvlTaw, size_t lvlSwild, size_t numberProbabilityManipulation, size_t has_cheer)
    {
        std::vector<Card> cards;

        if (lvlsamesis > 0)
        {
            cards.push_back(generateCard("samesis", lvlsamesis));
        }
        if (lvlTip > 0)
        {
            cards.push_back(generateCard("tipit", lvlTip));
        }

        if (lvlsixit > 0)
        {
            cards.push_back(generateCard("sixit", lvlsixit));
        }
        if (lvlWild > 0)
        {
            cards.push_back(generateCard("wild", lvlWild));
        }

        if (lvlTaw > 0)
        {
            cards.push_back(generateCard("twice as wild", lvlTaw));
        }

        if (lvlSwild > 0)
        {
            cards.push_back(generateCard("slightly wild", lvlSwild));
        }

        for (size_t i = 0; i< numberProbabilityManipulation; i++)
        {
            cards.push_back(generateCard("probability manipulation", 1));
        }

        if (has_cheer > 0)
        {
            cards.push_back(generateCard("cheer", 1));
        }

        return cards;
    }

    static std::string get_card_string(const Card& card)
    {
        return std::to_string(card.card_id) + " " + std::to_string(card.lvl);
    }

    static std::string get_cards_string(const std::vector<Card>& cards)
    {
        std::string ret = ";";
        std::vector<Card> cardstemp = cards;
        std::sort(cardstemp.begin(), cardstemp.end(), [](Card a, Card b) {
            return a.card_id < b.card_id;
            });
        for (const auto& c : cardstemp)
        {
            ret += get_card_string(c) + ";";
        }
        return ret;

    }
    
    sqlite3* db = nullptr;
    std::string last_db_path = "";

    sqlite3* matrixdb = nullptr;
    std::string last_matrixdb_path = "";

    std::string matrix_db_save_string = "";

};
