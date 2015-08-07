#include <gtest/gtest.h>

#include <memory>
#include <random>
#include <string>

#include <boost/filesystem.hpp>

#include "fsfixture.h"
#include "priority.pb.h"
#include "prioritybuffer.h"

#ifndef NUMBER_MESSAGES_IN_TEST
#define NUMBER_MESSAGES_IN_TEST 1000
#endif


namespace fs = ::boost::filesystem;

unsigned long long get_priority(const PriorityMessage& message) {
    return message.priority();
}

TEST_F(FSFixture, RandomPriorityTest) {
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
    unsigned long long priority = 100LL;
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        auto message = buffer.Pop();
        EXPECT_TRUE(message->IsInitialized());
        EXPECT_GE(priority, message->priority());
        priority = message->priority();
    }
}

TEST_F(FSFixture, MaxSizePriorityTest) {
    // Each message takes 2 bytes to store. At NUMBER_MESSAGES_IN_TEST max byte size, we can store
    // half of all messages on disk, and DEFAULT_MAX_MEMORY_SIZE messages in memory, for a total of
    // NUMBER_MESSAGES_IN_TEST / 2 + DEFAULT_MAX_MEMORY_SIZE.
    PriorityBuffer<PriorityMessage> buffer{get_priority, NUMBER_MESSAGES_IN_TEST,
                                           DEFAULT_MAX_MEMORY_SIZE};
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
    unsigned long long priority = 100LL;
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST / 2 + DEFAULT_MAX_MEMORY_SIZE; ++i) {
        auto message = buffer.Pop();
        EXPECT_TRUE(message->IsInitialized());
        EXPECT_GE(priority, message->priority());
        priority = message->priority();
    }

    // One more attempt should give us a message that is uninitialized.
    EXPECT_EQ(nullptr, buffer.Pop());
}

TEST_F(FSFixture, NoMemoryPriorityTest) {
    // Each message takes 2 bytes to store. At NUMBER_MESSAGES_IN_TEST max byte size, we can store
    // half of all messages on disk, and 0 messages in memory, for a total of
    // NUMBER_MESSAGES_IN_TEST / 2.
    PriorityBuffer<PriorityMessage> buffer{get_priority, NUMBER_MESSAGES_IN_TEST, 0};
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
    unsigned long long priority = 100LL;
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST / 2; ++i) {
        auto message = buffer.Pop();
        EXPECT_TRUE(message->IsInitialized());
        EXPECT_GE(priority, message->priority());
        priority = message->priority();
    }

    EXPECT_EQ(nullptr, buffer.Pop());
}

TEST_F(FSFixture, DiskDumpAllPriorityTest) {
    auto buffer_path = fs::temp_directory_path() / fs::path{"prism_buffer"};
    {
        // On destruction, PriorityBuffer will convert all in-memory objects to file objects
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
        // Since there are DEFAULT_MAX_MEMORY_SIZE in memory, there should be
        // NUMBER_MESSAGES_IN_TEST - DEFAULT_MAX_MEMORY_SIZE files in the default buffer directory.
        EXPECT_EQ(number_of_files_(), NUMBER_MESSAGES_IN_TEST - DEFAULT_MAX_MEMORY_SIZE);
    }

    EXPECT_EQ(number_of_files_(), NUMBER_MESSAGES_IN_TEST);
}

TEST_F(FSFixture, DiskDumpSomePriorityTest) {
    // Pop off some random number of messages
    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(0, 100LL);
    auto number_of_popped = distribution(generator);
    auto buffer_path = fs::temp_directory_path() / fs::path{"prism_buffer"};
    {
        PriorityBuffer<PriorityMessage> buffer{get_priority};
        for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
            auto message = std::unique_ptr<PriorityMessage>{ new PriorityMessage{} };
            auto priority = distribution(generator);
            message->set_priority(priority);
            EXPECT_TRUE(message->IsInitialized());
            EXPECT_EQ(priority, message->priority());
            buffer.Push(std::move(message));
        }
        unsigned long long priority = 100LL;
        for (int i = 0; i < number_of_popped; ++i) {
            auto message = buffer.Pop();
            EXPECT_TRUE(message->IsInitialized());
            EXPECT_GE(priority, message->priority());
            priority = message->priority();
        }
        auto disk_popped = DEFAULT_MAX_MEMORY_SIZE > number_of_popped ?
                           DEFAULT_MAX_MEMORY_SIZE : number_of_popped;
        EXPECT_EQ(number_of_files_(), NUMBER_MESSAGES_IN_TEST - disk_popped);
    }

    EXPECT_EQ(number_of_files_(), NUMBER_MESSAGES_IN_TEST - number_of_popped);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
