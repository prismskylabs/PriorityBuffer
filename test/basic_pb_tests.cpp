#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "basic.pb.h"
#include "prioritybuffer.h"

#define NUM_ITEMS 1000


TEST(BasicProtobufTests, DefaultPriorityTest) {
    PriorityBuffer<Basic> basics;
    for (int i = 0; i < NUM_ITEMS; ++i) {
        Basic basic;
        basic.set_value(std::to_string(i));
        EXPECT_EQ(std::to_string(i), basic.value());
        basic.CheckInitialized();
        basics.Push(basic);
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    for (int i = NUM_ITEMS - 1; i >= 0; --i) {
        auto basic = basics.Pop();
        basic.CheckInitialized();
        EXPECT_EQ(std::to_string(i), basic.value());
    }
}

unsigned long long reverse_priority(const Basic& basic) {
    return 20000000000000000LL - std::chrono::steady_clock::now().time_since_epoch().count();
}

TEST(BasicProtobufTests, ReversePriorityTest) {
    PriorityBuffer<Basic> basics{reverse_priority};
    for (int i = 0; i < NUM_ITEMS; ++i) {
        Basic basic;
        basic.set_value(std::to_string(i));
        EXPECT_EQ(std::to_string(i), basic.value());
        basic.CheckInitialized();
        basics.Push(basic);
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    for (int i = 0; i < NUM_ITEMS; ++i) {
        auto basic = basics.Pop();
        basic.CheckInitialized();
        EXPECT_EQ(std::to_string(i), basic.value());
    }
}

#include <iostream>
TEST(BasicProtobufTests, OutOfOrderTest) {
    std::vector<unsigned long long> ordered_priorities{5, 3, 7, 1, 8, 2};
    int priority_at = 0;

    auto ordered_priority =
        [&ordered_priorities, &priority_at] (const Basic& basic) -> unsigned long long {
            return ordered_priorities[priority_at++];
        };

    PriorityBuffer<Basic> basics{ordered_priority};
    for (auto& priority : ordered_priorities) {
        Basic basic;
        basic.set_value(std::to_string(priority));
        EXPECT_EQ(std::to_string(priority), basic.value());
        basic.CheckInitialized();
        basics.Push(basic);
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    std::sort(ordered_priorities.rbegin(), ordered_priorities.rend());
    for (auto& priority : ordered_priorities) {
        auto basic = basics.Pop();
        basic.CheckInitialized();
        EXPECT_EQ(std::to_string(priority), basic.value());
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
