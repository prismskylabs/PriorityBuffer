#include "prioritydb.h"

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <sqlite3.h>


typedef std::map<std::string, std::string> Record;

static int callback(void* response_ptr, int num_values, char** values, char** names) {
    auto response = (std::vector<Record>*) response_ptr;
    auto record = Record();
    for (int i = 0; i < num_values; ++i) {
        record[names[i]] = values[i];
    }

    response->push_back(record);

    return 0;
}

class PriorityDB::Impl {
  public:
    Impl(const std::string& path);

    void Insert(const unsigned long long& priority, const std::string& hash, const bool& on_disk);
    void Delete(const std::string& hash);
    std::string GetHighestHash();

  private:
    std::unique_ptr<sqlite3, std::function<int(sqlite3*)>> db_;
    std::string table_name_;

    bool check_table();
    void create_table();
    std::vector<Record> execute(const std::string& sql);
};

PriorityDB::Impl::Impl(const std::string& path) {
    sqlite3* db;
    sqlite3_open(path.data(), &db);
    db_ = std::unique_ptr<sqlite3, std::function<int(sqlite3*)>>(db, sqlite3_close);
    table_name_ = "prism_data";
    if (!check_table()) {
        create_table();
    }
    check_table();
}

void PriorityDB::Impl::Insert(const unsigned long long& priority, const std::string& hash,
                              const bool& on_disk) {
    std::stringstream stream;
    stream << "INSERT INTO "
           << table_name_
           << "(priority, hash, on_disk)"
           << "VALUES"
           << "("
           << priority << ","
           << "'" << hash << "',"
           << on_disk
           << ");";
    execute(stream.str());
}

void PriorityDB::Impl::Delete(const std::string& hash) {
    std::stringstream stream;
    stream << "DELETE FROM "
           << table_name_
           << " WHERE hash='"
           << hash
           << "';";
    execute(stream.str());
}

std::string PriorityDB::Impl::GetHighestHash() {
    std::stringstream stream;
    stream << "SELECT hash FROM "
           << table_name_
           << " ORDER BY priority DESC LIMIT 1;";
    auto response = execute(stream.str());
    std::string hash;
    if (!response.empty()) {
        auto record = response[0];
        if (!record.empty()) {
            hash = record["hash"];
        }
    }

    return hash;
}

bool PriorityDB::Impl::check_table() {
    std::stringstream stream;
    stream << "SELECT name FROM sqlite_master WHERE type='table' AND name='"
           << table_name_
           << "';";
    auto response = execute(stream.str());

    return !response.empty();
}

void PriorityDB::Impl::create_table() {
    std::stringstream stream;
    stream << "CREATE TABLE "
           << table_name_
           << "("
           << "id INTEGER PRIMARY KEY AUTOINCREMENT,"
           << "priority UNSIGNED BIGINT NOT NULL,"
           << "hash TEXT NOT NULL,"
           << "on_disk BOOL NOT NULL"
           << ");";
    execute(stream.str());
}

std::vector<Record>PriorityDB::Impl::execute(const std::string& sql) {
    using namespace std::placeholders;
    std::vector<Record> response;
    char* error;
    int rc = sqlite3_exec(db_.get(), sql.data(), callback, &response, &error);
    if (rc != SQLITE_OK) {
        std::cout << "Error: " << error << std::endl;
        sqlite3_free(error);
    }

    return response;
}


// Bridge

PriorityDB::PriorityDB(const std::string& path) : pimpl_{ new Impl{path} } {}
PriorityDB::~PriorityDB() {}

void PriorityDB::Insert(const unsigned long long& priority, const std::string& hash,
                        const bool& on_disk) {
    pimpl_->Insert(priority, hash, on_disk);
}

void PriorityDB::Delete(const std::string& hash) {
    pimpl_->Delete(hash);
}

std::string PriorityDB::GetHighestHash() {
    return pimpl_->GetHighestHash();
}
