#include <gtest/gtest.h>

#include <fstream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include <boost/filesystem.hpp>

#include "dbfixture.h"
#include "priority.pb.h"
#include "prioritybuffer.h"

#ifndef NUMBER_MESSAGES_IN_TEST
#define NUMBER_MESSAGES_IN_TEST 1000
#endif


namespace fs = ::boost::filesystem;

unsigned long long get_priority(const PriorityMessage& message) {
    return message.priority();
}

class FailureFixture : public DBFixture {
  protected:
    virtual void SetUp() {
        DBFixture::SetUp();
        {
            // Create the db so there's a table we can use
            PriorityDB db{DEFAULT_MAX_BUFFER_SIZE, db_string_};
        }
    }
};

TEST_F(FailureFixture, DeleteSomeDiskMessagesTest) {
    PriorityBuffer<PriorityMessage> buffer{get_priority};
    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(0, 100LL);
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        auto message = std::unique_ptr<PriorityMessage>{ new PriorityMessage{} };
        auto priority = distribution(generator);
        message->set_priority(priority);
        EXPECT_TRUE(message->IsInitialized());
        EXPECT_EQ(priority, message->priority());
        buffer.Push(std::move(message));
    }
    unsigned long long number_to_delete = 0;
    unsigned long long number_of_files = 0;
    {
        number_of_files = number_of_files_();
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
        ASSERT_EQ(number_of_files - number_to_delete, number_of_files_());
    }

    unsigned long long priority = 100LL;
    for (int i = 0; i < (NUMBER_MESSAGES_IN_TEST - number_to_delete); ) {
        auto message = buffer.Pop();
        // Check if the message has been deleted or not
        if (message) {
            // If it is, add it to i
            EXPECT_GE(priority, message->priority());
            ++i;
            priority = message->priority();
        }
    }

    // There should be no more initialized messages, so every subsequent Pop should be bad
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        auto message = buffer.Pop();
        if (!message) {
            break;
        }
        std::cout << "i: " << message.get() << std::endl;
    }
}

TEST_F(FailureFixture, ExistingDiskMessageTest) {
    PriorityBuffer<PriorityMessage> buffer{get_priority};
    
    // Push DEFAULT_MAX_MEMORY_SIZE messages into he buffer with 0 priority
    for (int i = 0; i < DEFAULT_MAX_MEMORY_SIZE; ++i) {
        auto message = std::unique_ptr<PriorityMessage>{ new PriorityMessage{} };
        message->set_priority(0);
        ASSERT_TRUE(message->IsInitialized());
        buffer.Push(std::move(message));
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

    // Push DEFAULT_MAX_MEMORY_SIZE messages into he buffer with 1 priority, pushing all the
    // previous messages out
    for (int i = 0; i < DEFAULT_MAX_MEMORY_SIZE; ++i) {
        auto message = std::unique_ptr<PriorityMessage>{ new PriorityMessage{} };
        message->set_priority(1);
        ASSERT_TRUE(message->IsInitialized());
        buffer.Push(std::move(message));
    }

    for (int i = 0; i < 100 - number_to_create; ++i) {
        auto message = buffer.Pop();
        ASSERT_TRUE(message->IsInitialized());
        ASSERT_GE(1, message->priority());
    }

    EXPECT_EQ(nullptr, buffer.Pop());
}

TEST_F(FailureFixture, ExistingDiskMessageOnDestructTest) {
    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(1, DEFAULT_MAX_MEMORY_SIZE);
    auto number_to_create = distribution(generator);

    {
        PriorityBuffer<PriorityMessage> buffer{get_priority};
        
        // Push DEFAULT_MAX_MEMORY_SIZE messages into the buffer with 0 priority
        for (int i = 0; i < DEFAULT_MAX_MEMORY_SIZE; ++i) {
            auto message = std::unique_ptr<PriorityMessage>{ new PriorityMessage{} };
            message->set_priority(0);
            ASSERT_TRUE(message->IsInitialized());
            buffer.Push(std::move(message));
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
    EXPECT_EQ(DEFAULT_MAX_MEMORY_SIZE - number_to_create, number_of_files_());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
