#include <gtest/gtest.h>

#include <map>
#include <string>

#include <boost/filesystem.hpp>
#include <sqlite3.h>

#include "fsfixture.h"
#include "prioritydb.h"


namespace fs = ::boost::filesystem;

class DBFixture : public FSFixture {
  protected:
    typedef std::map<std::string, std::string> Record;

    virtual void SetUp() {
        FSFixture::SetUp();
        fs::create_directory(buffer_path_);
        db_path_ = buffer_path_ / fs::path{"prism_data.db"};
        db_string_ = db_path_.native();
        table_name_ = "prism_data";
    }

    sqlite3* open_db_() {
        sqlite3* sqlite_db;
        if (sqlite3_open(db_string_.data(), &sqlite_db) != SQLITE_OK) {
            throw PriorityDBException{sqlite3_errmsg(sqlite_db)};
        }
        return sqlite_db;
    }

    int close_db_(sqlite3* db) {
        return sqlite3_close(db);
    }

    static int callback_(void* response_ptr, int num_values, char** values, char** names) {
        auto response = (std::vector<Record>*) response_ptr;
        auto record = Record();
        for (int i = 0; i < num_values; ++i) {
            if (values[i]) {
                record[names[i]] = values[i];
            }
        }

        response->push_back(record);

        return 0;
    }

    std::vector<Record> execute_(const std::string& sql) {
        std::vector<Record> response;
        auto db = open_db_();
        char* error;
        int rc = sqlite3_exec(db, sql.data(), &callback_, &response, &error);
        if (rc != SQLITE_OK) {
            auto error_string = std::string{error};
            sqlite3_free(error);
            throw PriorityDBException{error_string};
        }

        return response;
    }

    fs::path db_path_;
    std::string db_string_;
    std::string table_name_;
};
