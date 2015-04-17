#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>
#include <sqlite3.h>

#include "bufferfixture.h"
#include "priority.pb.h"
#include "prioritybuffer.h"

#ifndef NUMBER_MESSAGES_IN_TEST
#define NUMBER_MESSAGES_IN_TEST 1000
#endif


namespace fs = boost::filesystem;

unsigned long long get_priority(const PriorityMessage& message) {
    return message.priority();
}

class FailureFixture : public BufferFixture {
  protected:
    typedef std::map<std::string, std::string> Record;

    virtual void SetUp() {
        BufferFixture::SetUp();
        buffer_path_ = fs::temp_directory_path() / fs::path{"prism_buffer"};
        fs::create_directory(buffer_path_);
        db_path_ = fs::temp_directory_path() / fs::path{"prism_buffer"} / fs::path{"prism_data.db"};
        db_string_ = db_path_.native();
        table_name_ = "prism_data";
        {
            // Create the db so there's a table we can use
            PriorityDB db{DEFAULT_MAX_BUFFER_SIZE, db_string_};
        }
    }

    virtual void TearDown() {
        BufferFixture::TearDown();
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
    fs::path buffer_path_;
    std::string db_string_;
    std::string table_name_;
};

TEST_F(FailureFixture, DeleteSomeDiskMessagesTest) {
    PriorityBuffer<PriorityMessage> buffer{get_priority};
    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(0, 100LL);
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        PriorityMessage message;
        auto priority = distribution(generator);
        message.set_priority(priority);
        EXPECT_TRUE(message.IsInitialized());
        EXPECT_EQ(priority, message.priority());
        buffer.Push(message);
    }
    unsigned long long number_to_delete = 0;
    unsigned long long number_of_files = 0;
    {
        fs::directory_iterator begin(buffer_path_), end;
        number_of_files = std::count_if(begin, end,
                [] (const fs::directory_entry& f) {
                    return !(fs::is_directory(f.path()) ||
                             f.path().filename().native().substr(0, 10) == "prism_data");
                });

        std::uniform_int_distribution<unsigned long long> random_delete(1, number_of_files);
        number_to_delete = random_delete(generator);
    }

    fs::directory_iterator begin(buffer_path_), end;
    std::vector<std::string> hashes;
    for (auto iterator = begin; iterator != end; ++iterator) {
        if (!(fs::is_directory(*iterator) || 
                iterator->path().filename().native().substr(0, 10) == "prism_data")) {
            if (hashes.size() >= number_to_delete) {
                break;
            }

            hashes.push_back(iterator->path().filename().native());
        }
    }

    for (auto& hash : hashes) {
        ASSERT_TRUE(fs::remove(buffer_path_ / fs::path{hash}));
    }
    {
        fs::directory_iterator begin(buffer_path_), end;
        auto new_number_of_files = std::count_if(begin, end,
                [] (const fs::directory_entry& f) {
                    return !(fs::is_directory(f.path()) ||
                             f.path().filename().native().substr(0, 10) == "prism_data");
                });
        ASSERT_EQ(number_of_files - number_to_delete, new_number_of_files);
    }

    unsigned long long priority = 100LL;
    for (int i = 0; i < (NUMBER_MESSAGES_IN_TEST - number_to_delete); ) {
        auto message = buffer.Pop();
        // Check if the message has been deleted or not
        if (message.IsInitialized()) {
            // If it is, add it to i
            EXPECT_GE(priority, message.priority());
            ++i;
            priority = message.priority();
        }
    }

    // There should be no more initialized messages, so every subsequent Pop should be bad
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        EXPECT_FALSE(buffer.Pop().IsInitialized());
    }
}

TEST_F(FailureFixture, ExistingDiskMessageTest) {
    PriorityBuffer<PriorityMessage> buffer{get_priority};
    
    // Push 50 messages into he buffer with 0 priority
    for (int i = 0; i < 50; ++i) {
        PriorityMessage message;
        message.set_priority(0);
        ASSERT_TRUE(message.IsInitialized());
        buffer.Push(message);
    }

    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(1, DEFAULT_MAX_MEMORY_SIZE);
    auto number_to_create = distribution(generator);

    std::stringstream stream;
    stream << "SELECT hash FROM "
           << table_name_
           << " ORDER BY priority LIMIT "
           << number_to_create
           << ";";
    auto response = execute_(stream.str());
    ASSERT_EQ(number_to_create, response.size());

    for (auto& record : response) {
        auto file_path = buffer_path_ / fs::path{record["hash"]};
        std::ofstream file_out{file_path.native()};
        file_out << "hello world";
    }

        PriorityMessage message;
        message.set_priority(1);
        ASSERT_TRUE(message.IsInitialized());
        buffer.Push(message);
    // Push DEFAULT_MAX_MEMORY_SIZE messages into he buffer with 1 priority, pushing all the
    // previous messages out
    for (int i = 0; i < DEFAULT_MAX_MEMORY_SIZE; ++i) {
    }

    for (int i = 0; i < 100 - number_to_create; ++i) {
        auto message = buffer.Pop();
        ASSERT_TRUE(message.IsInitialized());
        ASSERT_GE(1, message.priority());
    }

    EXPECT_FALSE(buffer.Pop().IsInitialized());
}

TEST_F(FailureFixture, ExistingDiskMessageOnDestructTest) {
    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(1, DEFAULT_MAX_MEMORY_SIZE);
    auto number_to_create = distribution(generator);

    {
        PriorityBuffer<PriorityMessage> buffer{get_priority};
        
            PriorityMessage message;
            message.set_priority(0);
            ASSERT_TRUE(message.IsInitialized());
            buffer.Push(message);
        // Push DEFAULT_MAX_MEMORY_SIZE messages into he buffer with 0 priority
        for (int i = 0; i < DEFAULT_MAX_MEMORY_SIZE; ++i) {
        }

        std::stringstream stream;
        stream << "SELECT hash FROM "
               << table_name_
               << " ORDER BY priority LIMIT "
               << number_to_create
               << ";";
        auto response = execute_(stream.str());
        ASSERT_EQ(number_to_create, response.size());

        for (auto& record : response) {
            auto file_path = buffer_path_ / fs::path{record["hash"]};
            std::ofstream file_out{file_path.native()};
            file_out << "hello world";
        }
    }

    // Let the buffer drop and try to push memory messages to disk
    fs::directory_iterator begin(buffer_path_), end;
    int number_of_files = std::count_if(begin, end,
            [] (const fs::directory_entry& f) {
                return !(fs::is_directory(f.path()) ||
                         f.path().filename().native().substr(0, 10) == "prism_data");
            });

    EXPECT_EQ(DEFAULT_MAX_MEMORY_SIZE - number_to_create, number_of_files);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
