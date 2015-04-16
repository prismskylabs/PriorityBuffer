#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "basic.pb.h"
#include "bufferfixture.h"
#include "prioritybuffer.h"

#ifndef NUMBER_MESSAGES_IN_TEST
#define NUMBER_MESSAGES_IN_TEST 1000
#endif


TEST_F(BufferFixture, DefaultPriorityTest) {
    PriorityBuffer<Basic> basics;
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        Basic basic;
        basic.set_value(std::to_string(i));
        EXPECT_EQ(std::to_string(i), basic.value());
        EXPECT_TRUE(basic.IsInitialized());
        basics.Push(basic);
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    for (int i = NUMBER_MESSAGES_IN_TEST - 1; i >= 0; --i) {
        auto basic = basics.Pop();
        EXPECT_TRUE(basic.IsInitialized());
        EXPECT_EQ(std::to_string(i), basic.value());
    }
}

unsigned long long reverse_priority(const Basic& basic) {
    return 20000000000000000LL - std::chrono::steady_clock::now().time_since_epoch().count();
}

TEST_F(BufferFixture, ReversePriorityTest) {
    PriorityBuffer<Basic> basics{reverse_priority};
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        Basic basic;
        basic.set_value(std::to_string(i));
        EXPECT_TRUE(basic.IsInitialized());
        EXPECT_EQ(std::to_string(i), basic.value());
        basics.Push(basic);
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    for (int i = 0; i < NUMBER_MESSAGES_IN_TEST; ++i) {
        auto basic = basics.Pop();
        EXPECT_TRUE(basic.IsInitialized());
        EXPECT_EQ(std::to_string(i), basic.value());
    }
}

TEST_F(BufferFixture, OutOfOrderTest) {
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
        EXPECT_TRUE(basic.IsInitialized());
        EXPECT_EQ(std::to_string(priority), basic.value());
        basics.Push(basic);
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
    std::sort(ordered_priorities.rbegin(), ordered_priorities.rend());
    for (auto& priority : ordered_priorities) {
        auto basic = basics.Pop();
        EXPECT_TRUE(basic.IsInitialized());
        EXPECT_EQ(std::to_string(priority), basic.value());
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
