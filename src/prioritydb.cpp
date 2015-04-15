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
        if (values[i]) {
            record[names[i]] = values[i];
        }
    }

    response->push_back(record);

    return 0;
}

class PriorityDB::Impl {
  public:
    Impl(const unsigned long long& max_size) : max_size_{max_size} {}

    int Open(const std::string& path);
    void Insert(const unsigned long long& priority, const std::string& hash,
                const unsigned long long& size, const bool& on_disk);
    void Delete(const std::string& hash);
    void Update(const std::string& hash, const bool& on_disk);
    std::string GetHighestHash(bool& on_disk);
    std::string GetLowestMemoryHash();
    std::string GetLowestDiskHash();
    bool Full();

  private:
    std::unique_ptr<sqlite3, std::function<int(sqlite3*)>> db_;
    std::string table_name_;
    unsigned long long max_size_;

    bool check_table();
    void create_table();
    std::vector<Record> execute(const std::string& sql);
};

int PriorityDB::Impl::Open(const std::string& path) {
    sqlite3* db;
    sqlite3_open(path.data(), &db);
    db_ = std::unique_ptr<sqlite3, std::function<int(sqlite3*)>>(db, sqlite3_close);
    table_name_ = "prism_data";
    if (!check_table()) {
        create_table();
    }
    check_table();

    return 0;
}

void PriorityDB::Impl::Insert(const unsigned long long& priority, const std::string& hash,
                              const unsigned long long& size, const bool& on_disk) {
    std::stringstream stream;
    stream << "INSERT INTO "
           << table_name_
           << "(priority, hash, size, on_disk)"
           << "VALUES"
           << "("
           << priority << ","
           << "'" << hash << "',"
           << size << ","
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

void PriorityDB::Impl::Update(const std::string& hash, const bool& on_disk) {
    std::stringstream stream;
    stream << "UPDATE "
           << table_name_
           << " SET on_disk="
           << on_disk
           << " WHERE hash='"
           << hash
           << "';";
    execute(stream.str());
}

std::string PriorityDB::Impl::GetHighestHash(bool& on_disk) {
    std::stringstream stream;
    stream << "SELECT hash, on_disk FROM "
           << table_name_
           << " ORDER BY priority DESC LIMIT 1;";
    auto response = execute(stream.str());
    std::string hash;
    if (!response.empty()) {
        auto record = response[0];
        if (!record.empty()) {
            hash = record["hash"];
            on_disk = std::stoi(record["on_disk"]);
        }
    }

    return hash;
}

std::string PriorityDB::Impl::GetLowestMemoryHash() {
    std::stringstream stream;
    stream << "SELECT hash FROM "
           << table_name_
           << " WHERE on_disk="
           << false
           << " ORDER BY priority ASC LIMIT 1;";
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

std::string PriorityDB::Impl::GetLowestDiskHash() {
    std::stringstream stream;
    stream << "SELECT hash FROM "
           << table_name_
           << " WHERE on_disk="
           << true
           << " ORDER BY priority ASC LIMIT 1;";
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

bool PriorityDB::Impl::Full() {
    std::stringstream stream;
    stream << "SELECT SUM(size) FROM "
           << table_name_
           << " WHERE on_disk="
           << true
           << ";";
    auto response = execute(stream.str());
    unsigned long long total = 0;
    if (!response.empty()) {
        auto record = response[0];
        if (!record.empty()) {
            total = std::stoi(record["SUM(size)"]);
        }
    }

    return total > max_size_;
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
           << "size UNSIGNED BIGINT NOT NULL,"
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

PriorityDB::PriorityDB(const unsigned long long& max_size) : pimpl_{ new Impl{max_size} } {}
PriorityDB::~PriorityDB() {}

int PriorityDB::Open(const std::string& path) {
    return pimpl_->Open(path);
}

void PriorityDB::Insert(const unsigned long long& priority, const std::string& hash,
                        const unsigned long long& size, const bool& on_disk) {
    pimpl_->Insert(priority, hash, size, on_disk);
}

void PriorityDB::Delete(const std::string& hash) {
    pimpl_->Delete(hash);
}

void PriorityDB::Update(const std::string& hash, const bool& on_disk) {
    pimpl_->Update(hash, on_disk);
}

std::string PriorityDB::GetHighestHash(bool& on_disk) {
    return pimpl_->GetHighestHash(on_disk);
}

std::string PriorityDB::GetLowestMemoryHash() {
    return pimpl_->GetLowestMemoryHash();
}

std::string PriorityDB::GetLowestDiskHash() {
    return pimpl_->GetLowestDiskHash();
}

bool PriorityDB::Full() {
    return pimpl_->Full();
}
