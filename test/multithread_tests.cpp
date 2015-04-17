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


namespace fs = boost::filesystem;

unsigned long long get_priority(const PriorityMessage& message) {
    return message.priority();
}

void push(PriorityBuffer<PriorityMessage>& buffer) {
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
}

void pull(PriorityBuffer<PriorityMessage>& buffer) {
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ) {
        if (buffer.Pop()) {
            ++i;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_EQ(nullptr, buffer.Pop());
}

void pull_block(PriorityBuffer<PriorityMessage>& buffer) {
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        auto message = buffer.Pop(true);
        EXPECT_TRUE(message->IsInitialized());
    }
    EXPECT_EQ(nullptr, buffer.Pop());
}

TEST_F(FSFixture, RandomMultithreadedTest) {
    PriorityBuffer<PriorityMessage> buffer{get_priority};

    std::thread push_thread(push, std::ref(buffer));
    std::thread pull_thread(push, std::ref(buffer));

    push_thread.join();
    pull_thread.join();
}

TEST_F(FSFixture, RandomMultithreadedWithBlockingTest) {
    PriorityBuffer<PriorityMessage> buffer{get_priority};

    std::thread pull_thread(pull_block, std::ref(buffer));
    std::thread push_thread(push, std::ref(buffer));

    push_thread.join();
    pull_thread.join();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
