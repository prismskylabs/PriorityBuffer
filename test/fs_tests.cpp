#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>

#include "fsfixture.h"
#include "priorityfs.h"


namespace pb = ::prism::prioritybuffer;

TEST_F(FSFixture, EmptyFSTest) {
    EXPECT_FALSE(fs::exists(buffer_path_));
}

TEST_F(FSFixture, ConstructFSTest) {
    ASSERT_FALSE(fs::exists(buffer_path_));
    pb::PriorityFS priority_fs{"prism_buffer"};
    EXPECT_TRUE(fs::exists(buffer_path_));
}

TEST_F(FSFixture, ConstructFSNoDestructTest) {
    {
        pb::PriorityFS priority_fs{"prism_buffer"};
        EXPECT_TRUE(fs::exists(buffer_path_));
    }
    EXPECT_TRUE(fs::exists(buffer_path_));
}

TEST_F(FSFixture, ConstructFSMultipleTest) {
    ASSERT_FALSE(fs::exists(buffer_path_));
    {
        pb::PriorityFS priority_fs{"prism_buffer"};
        ASSERT_TRUE(fs::exists(buffer_path_));
    }
    {
        pb::PriorityFS priority_fs{"prism_buffer"};
        EXPECT_TRUE(fs::exists(buffer_path_));
    }
    EXPECT_TRUE(fs::exists(buffer_path_));
}

TEST_F(FSFixture, ConstructThrowTest) {
    bool thrown = false;
    try {
        pb::PriorityFS priority_fs{""};
    } catch (const pb::PriorityFSException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"Cannot initialize PriorityFS with an empty buffer path"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(FSFixture, ConstructCurrentThrowTest) {
    bool thrown = false;
    try {
        pb::PriorityFS priority_fs{"."};
    } catch (const pb::PriorityFSException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"PriorityFS must be initialized within a valid parent directory"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(FSFixture, ConstructParentThrowTest) {
    bool thrown = false;
    try {
        pb::PriorityFS priority_fs{".."};
    } catch (const pb::PriorityFSException& e) {
        thrown = true;
        EXPECT_EQ(std::string{"PriorityFS must be initialized within a valid parent directory"},
                  std::string{e.what()});
    }
    EXPECT_TRUE(thrown);
}

TEST_F(FSFixture, InitialEmptyFSTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    EXPECT_TRUE(fs::exists(buffer_path_));
    EXPECT_EQ(0, number_of_files_());
}

TEST_F(FSFixture, GetFilePathTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    auto path_string = priority_fs.GetFilePath("file");
    EXPECT_EQ(fs::path{path_string}, buffer_path_ / fs::path{"file"});
}

TEST_F(FSFixture, GetFilePathEmptyTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    auto path_string = priority_fs.GetFilePath("");
    EXPECT_EQ(fs::path{path_string}, buffer_path_);
}

TEST_F(FSFixture, GetFilePathCurrentTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    auto path_string = priority_fs.GetFilePath(".");
    EXPECT_EQ(fs::path{path_string}, buffer_path_ / fs::path{"."});
}

TEST_F(FSFixture, GetInputUnopenedTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ifstream stream;
    EXPECT_FALSE(priority_fs.GetInput("file", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetInputEmptyTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ifstream stream;
    EXPECT_FALSE(priority_fs.GetInput("", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetInputCurrentTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ifstream stream;
    EXPECT_FALSE(priority_fs.GetInput(".", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetInputParentTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ifstream stream;
    EXPECT_FALSE(priority_fs.GetInput("..", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetInputOpenedTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    {
        std::ofstream out_stream{(buffer_path_ / fs::path{"file"}).native()};
        out_stream << "hello world";
    }
    std::ifstream stream;
    EXPECT_TRUE(priority_fs.GetInput("file", stream));
    EXPECT_TRUE(stream.is_open());
    stream.close();
}

TEST_F(FSFixture, GetInputReadTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    {
        std::ofstream out_stream{(buffer_path_ / fs::path{"file"}).native()};
        out_stream << "hello world";
    }
    std::ifstream stream;
    ASSERT_TRUE(priority_fs.GetInput("file", stream));
    ASSERT_TRUE(stream.is_open());
    std::string read((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    stream.close();
    EXPECT_EQ(std::string{"hello world"}, read);
}

TEST_F(FSFixture, GetOutputNewTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ofstream stream;
    ASSERT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
    EXPECT_TRUE(priority_fs.GetOutput("file", stream));
    EXPECT_TRUE(stream.is_open());
    stream.close();
}

TEST_F(FSFixture, GetOutputExistingTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    {
        std::ofstream out_stream{(buffer_path_ / fs::path{"file"}).native()};
        out_stream << "hello world";
    }
    std::ofstream stream;
    ASSERT_TRUE(fs::exists(buffer_path_ / fs::path{"file"}));
    EXPECT_FALSE(priority_fs.GetOutput("file", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetOutputEmptyTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ofstream stream;
    EXPECT_FALSE(priority_fs.GetOutput("", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetOutputCurrentTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ofstream stream;
    EXPECT_FALSE(priority_fs.GetOutput(".", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetOutputParentTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ofstream stream;
    EXPECT_FALSE(priority_fs.GetOutput("..", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetOutputWriteTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    std::ofstream stream;
    ASSERT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
    ASSERT_TRUE(priority_fs.GetOutput("file", stream));
    ASSERT_TRUE(stream.is_open());
    stream << "hello world";
    stream.close();

    std::ifstream in_stream{(buffer_path_ / fs::path{"file"}).native()};
    std::string read((std::istreambuf_iterator<char>(in_stream)), std::istreambuf_iterator<char>());
    EXPECT_EQ(std::string{"hello world"}, read);
}

TEST_F(FSFixture, DeleteFalseTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    ASSERT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
    EXPECT_FALSE(priority_fs.Delete("file"));
    EXPECT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
}

TEST_F(FSFixture, DeleteFalseNullFileTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    EXPECT_FALSE(priority_fs.Delete(""));
    EXPECT_TRUE(fs::exists(buffer_path_ / fs::path{""}));
}

TEST_F(FSFixture, DeleteFalseRelativeTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    EXPECT_FALSE(priority_fs.Delete(".."));
    EXPECT_TRUE(fs::exists(buffer_path_ / fs::path{".."}));
}

TEST_F(FSFixture, DeleteTrueTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    {
        std::ofstream out_stream{(buffer_path_ / fs::path{"file"}).native()};
        out_stream << "hello world";
    }
    ASSERT_TRUE(fs::exists(buffer_path_ / fs::path{"file"}));
    EXPECT_TRUE(priority_fs.Delete("file"));
    EXPECT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
}

TEST_F(FSFixture, DeleteTrueRelativeTest) {
    pb::PriorityFS priority_fs{"prism_buffer"};
    {
        std::ofstream out_stream{(buffer_path_ / fs::path{"file"}).native()};
        out_stream << "hello world";
    }
    ASSERT_TRUE(fs::exists(buffer_path_ / fs::path{"../prism_buffer/file"}));
    EXPECT_TRUE(priority_fs.Delete("../prism_buffer/file"));
    EXPECT_FALSE(fs::exists(buffer_path_ / fs::path{"../prism_buffer/file"}));
}
