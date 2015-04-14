#include <gtest/gtest.h>

#include <chrono>
#include <iostream>
#include <random>
#include <string>
#include <thread>

#include "priority.pb.h"
#include "prioritybuffer.h"

#define NUM_ITEMS 1000

unsigned long long get_priority(const PriorityMessage& message) {
    return message.priority();
}

TEST(PriorityProtobufTests, RandomPriorityTest) {
    PriorityBuffer<PriorityMessage> buffer{get_priority};
    std::random_device generator;
    std::uniform_int_distribution<unsigned long long> distribution(0, 100LL);
    for (int i = 0; i < NUM_ITEMS; ++i) {
        PriorityMessage message;
        auto priority = distribution(generator);
        message.set_priority(priority);
        EXPECT_EQ(priority, message.priority());
        message.CheckInitialized();
        buffer.Push(message);
    }
    unsigned long long priority = 100LL;
    for (int i = 0; i < NUM_ITEMS; ++i) {
        auto message = buffer.Pop();
        message.CheckInitialized();
        EXPECT_GE(priority, message.priority());
        priority = message.priority();
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
