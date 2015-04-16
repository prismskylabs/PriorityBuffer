#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <random>
#include <string>
#include <thread>

#include <boost/filesystem.hpp>

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

TEST_F(BufferFixture, RandomPriorityTest) {
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
    unsigned long long priority = 100LL;
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        auto message = buffer.Pop();
        EXPECT_TRUE(message.IsInitialized());
        EXPECT_GE(priority, message.priority());
        priority = message.priority();
    }
}

TEST_F(BufferFixture, MaxSizePriorityTest) {
    // Each message takes 2 bytes to store. At NUMBER_MESSAGES_IN_TEST max byte size, we can store
    // half of all messages on disk, and 50 messages in memory, for a total of
    // NUMBER_MESSAGES_IN_TEST / 2 + 50.
    PriorityBuffer<PriorityMessage> buffer{get_priority, NUMBER_MESSAGES_IN_TEST};
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
    unsigned long long priority = 100LL;
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST / 2 + 50; ++i) {
        auto message = buffer.Pop();
        EXPECT_TRUE(message.IsInitialized());
        EXPECT_GE(priority, message.priority());
        priority = message.priority();
    }

    // One more attempt should give us a message that is uninitialized.
    auto message = buffer.Pop();
    EXPECT_FALSE(message.IsInitialized());
    EXPECT_GE(priority, message.priority());
}

TEST_F(BufferFixture, NoMemoryPriorityTest) {
    // Each message takes 2 bytes to store. At NUMBER_MESSAGES_IN_TEST max byte size, we can store
    // half of all messages on disk, and 0 messages in memory, for a total of
    // NUMBER_MESSAGES_IN_TEST / 2.
    PriorityBuffer<PriorityMessage> buffer{get_priority, NUMBER_MESSAGES_IN_TEST, 0};
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
    unsigned long long priority = 100LL;
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST / 2; ++i) {
        auto message = buffer.Pop();
        EXPECT_TRUE(message.IsInitialized());
        EXPECT_GE(priority, message.priority());
        priority = message.priority();
    }

    // One more attempt should give us a message that is uninitialized.
    auto message = buffer.Pop();
    EXPECT_FALSE(message.IsInitialized());
    EXPECT_GE(priority, message.priority());
}

TEST_F(BufferFixture, DiskDumpAllPriorityTest) {
    auto buffer_path = fs::temp_directory_path() / fs::path{"prism_buffer"};
    {
        // On destruction, PriorityBuffer will convert all in-memory objects to file objects
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
        // Since there are 50 in memory, there should be NUMBER_MESSAGES_IN_TEST - 50 files in the
        // default buffer directory.
        
        fs::directory_iterator begin(buffer_path), end;
        int number_of_files = std::count_if(begin, end,
                [] (const fs::directory_entry& f) {
                    return !(fs::is_directory(f.path()) ||
                             f.path().filename().native().substr(0, 10) == "prism_data");
                });

        EXPECT_EQ(number_of_files, NUMBER_MESSAGES_IN_TEST - 50);
    }

    fs::directory_iterator begin(buffer_path), end;
    int number_of_files = std::count_if(begin, end,
            [] (const fs::directory_entry& f) {
                return !(fs::is_directory(f.path()) ||
                         f.path().filename().native().substr(0, 10) == "prism_data");
            });
    EXPECT_EQ(number_of_files, NUMBER_MESSAGES_IN_TEST);
}

TEST_F(BufferFixture, DiskDumpSomePriorityTest) {
    // Pop off some random number of messages
    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(0, 100LL);
    auto number_of_popped = distribution(generator);
    auto buffer_path = fs::temp_directory_path() / fs::path{"prism_buffer"};
    {
        PriorityBuffer<PriorityMessage> buffer{get_priority};
        for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
            PriorityMessage message;
            auto priority = distribution(generator);
            message.set_priority(priority);
            EXPECT_TRUE(message.IsInitialized());
            EXPECT_EQ(priority, message.priority());
            buffer.Push(message);
        }
        unsigned long long priority = 100LL;
        for (int i = 0; i < number_of_popped; ++i) {
            auto message = buffer.Pop();
            EXPECT_TRUE(message.IsInitialized());
            EXPECT_GE(priority, message.priority());
            priority = message.priority();
        }
        fs::directory_iterator begin(buffer_path), end;
        int number_of_files = std::count_if(begin, end,
                [] (const fs::directory_entry& f) {
                    return !(fs::is_directory(f.path()) ||
                             f.path().filename().native().substr(0, 10) == "prism_data");
                });

        auto disk_popped = 50 > number_of_popped ? 50 : number_of_popped;
        EXPECT_EQ(number_of_files, NUMBER_MESSAGES_IN_TEST - disk_popped);
    }

    fs::directory_iterator begin(buffer_path), end;
    int number_of_files = std::count_if(begin, end,
            [] (const fs::directory_entry& f) {
                return !(fs::is_directory(f.path()) ||
                         f.path().filename().native().substr(0, 10) == "prism_data");
            });
    EXPECT_EQ(number_of_files, NUMBER_MESSAGES_IN_TEST - number_of_popped);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
