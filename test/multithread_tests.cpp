#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <random>
#include <thread>

#include "fsfixture.h"
#include "priority.pb.h"
#include "prioritybuffer.h"

#ifndef NUMBER_MESSAGES_IN_TEST
#define NUMBER_MESSAGES_IN_TEST 1000
#endif


namespace fs = ::boost::filesystem;
namespace pb = ::prism::prioritybuffer;

unsigned long long get_priority(const PriorityMessage& message) {
    return message.priority();
}

void push(pb::PriorityBuffer<PriorityMessage>& buffer, int messages) {
    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(0, 100LL);
    for (int i = 0; i < messages; ++i) {
        auto message = std::unique_ptr<PriorityMessage>{ new PriorityMessage{} };
        auto priority = distribution(generator);
        message->set_priority(priority);
        EXPECT_TRUE(message->IsInitialized());
        EXPECT_EQ(priority, message->priority());
        buffer.Push(std::move(message));
    }
}

void pull(pb::PriorityBuffer<PriorityMessage>& buffer) {
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ) {
        if (buffer.Pop()) {
            ++i;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(nullptr, buffer.Pop());
}

void pull_block(pb::PriorityBuffer<PriorityMessage>& buffer, int messages) {
    for (int i = 0; i < messages; ++i) {
        auto message = buffer.Pop(true);
        EXPECT_TRUE(message->IsInitialized());
    }
    EXPECT_EQ(nullptr, buffer.Pop());
}

TEST_F(FSFixture, RandomMultithreadedTest) {
    pb::PriorityBuffer<PriorityMessage> buffer{get_priority};

    std::thread push_thread(push, std::ref(buffer), NUMBER_MESSAGES_IN_TEST);
    std::thread pull_thread(push, std::ref(buffer), NUMBER_MESSAGES_IN_TEST);

    push_thread.join();
    pull_thread.join();
}

TEST_F(FSFixture, RandomMultithreadedWithBlockingTest) {
    pb::PriorityBuffer<PriorityMessage> buffer{get_priority};

    std::thread pull_thread(pull_block, std::ref(buffer), NUMBER_MESSAGES_IN_TEST);
    std::thread push_thread(push, std::ref(buffer), NUMBER_MESSAGES_IN_TEST);

    push_thread.join();
    pull_thread.join();
}

TEST_F(FSFixture, RandomMultithreadedWithBlockingFuzzTest) {
    pb::PriorityBuffer<PriorityMessage> buffer{get_priority};
    buffer.SetFuzz(1000, 1100); // This test should take ~5 seconds

    auto start = std::chrono::system_clock::now();

    std::thread pull_thread(pull_block, std::ref(buffer), 5);
    std::thread push_thread(push, std::ref(buffer), 5);

    push_thread.join();
    pull_thread.join();

    auto end = std::chrono::system_clock::now();
    EXPECT_GT(end - start, std::chrono::seconds(5));
}

TEST_F(FSFixture, RandomMultithreadedWithBlockingNoFuzzTest) {
    pb::PriorityBuffer<PriorityMessage> buffer{get_priority};
    buffer.SetFuzz(0, 0);

    auto start = std::chrono::system_clock::now();

    std::thread pull_thread(pull_block, std::ref(buffer), 5);
    std::thread push_thread(push, std::ref(buffer), 5);

    push_thread.join();
    pull_thread.join();

    auto end = std::chrono::system_clock::now();
    EXPECT_LT(end - start, std::chrono::seconds(5));
}

TEST_F(FSFixture, RandomMultithreadedWithBlockingBadFuzzTest) {
    pb::PriorityBuffer<PriorityMessage> buffer{get_priority};
    buffer.SetFuzz(1000, 100);

    auto start = std::chrono::system_clock::now();

    std::thread pull_thread(pull_block, std::ref(buffer), 5);
    std::thread push_thread(push, std::ref(buffer), 5);

    push_thread.join();
    pull_thread.join();

    auto end = std::chrono::system_clock::now();
    EXPECT_LT(end - start, std::chrono::seconds(5));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
