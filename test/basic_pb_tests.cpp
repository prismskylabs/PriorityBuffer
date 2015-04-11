#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>

#include "basic.pb.h"


TEST(BasicProtobufTests, HelloWorldTest) {
    Basic basic;
    basic.set_value("Hello world");
    basic.CheckInitialized();
    EXPECT_EQ(std::string{"Hello world"}, basic.value());

    std::stringstream stream;
    basic.SerializeToOstream(&stream);

    Basic new_basic;
    new_basic.ParseFromIstream(&stream);
    EXPECT_EQ(std::string{"Hello world"}, new_basic.value());
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    return RUN_ALL_TESTS();
}
