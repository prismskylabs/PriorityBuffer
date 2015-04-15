#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>

#include <boost/filesystem.hpp>

#include "priorityfs.h"


namespace fs = boost::filesystem;

class FSFixture : public ::testing::Test {
  protected:
    virtual void SetUp() {
        buffer_path_ = fs::temp_directory_path() / fs::path{"prism_buffer"};
        fs::remove_all(buffer_path_);
    }

    virtual void TearDown() {
        fs::remove_all(buffer_path_);
    }

    int number_of_files(const fs::path& path) {
        fs::directory_iterator begin(path), end;
        return std::count_if(begin, end,
                [] (const fs::directory_entry& f) {
                    return !fs::is_directory(f.path());
                });
    }

    fs::path buffer_path_;
};

TEST_F(FSFixture, EmptyFSTest) {
    EXPECT_FALSE(fs::exists(buffer_path_));
}

TEST_F(FSFixture, ConstructFSTest) {
    PriorityFS priority_fs{"prism_buffer"};
    EXPECT_TRUE(fs::exists(buffer_path_));
}

TEST_F(FSFixture, ConstructFSNoDestructTest) {
    {
        PriorityFS priority_fs{"prism_buffer"};
        EXPECT_TRUE(fs::exists(buffer_path_));
    }
    EXPECT_TRUE(fs::exists(buffer_path_));
}

TEST_F(FSFixture, InitialEmptyFSTest) {
    PriorityFS priority_fs{"prism_buffer"};
    EXPECT_TRUE(fs::exists(buffer_path_));
    EXPECT_EQ(0, number_of_files(buffer_path_));
}

TEST_F(FSFixture, GetFilePathTest) {
    PriorityFS priority_fs{"prism_buffer"};
    auto path_string = priority_fs.GetFilePath("file");
    EXPECT_EQ(path_string, (buffer_path_ / fs::path{"file"}).native());
}

TEST_F(FSFixture, GetInputUnopenedTest) {
    PriorityFS priority_fs{"prism_buffer"};
    std::ifstream stream;
    EXPECT_FALSE(priority_fs.GetInput("file", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetInputOpenedTest) {
    PriorityFS priority_fs{"prism_buffer"};
    {
        auto out_stream = std::ofstream{(buffer_path_ / fs::path{"file"}).native()};
        out_stream << "hello world";
    }
    std::ifstream stream;
    EXPECT_TRUE(priority_fs.GetInput("file", stream));
    EXPECT_TRUE(stream.is_open());
    stream.close();
}

TEST_F(FSFixture, GetInputReadTest) {
    PriorityFS priority_fs{"prism_buffer"};
    {
        auto out_stream = std::ofstream{(buffer_path_ / fs::path{"file"}).native()};
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
    PriorityFS priority_fs{"prism_buffer"};
    std::ofstream stream;
    ASSERT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
    EXPECT_TRUE(priority_fs.GetOutput("file", stream));
    EXPECT_TRUE(stream.is_open());
    stream.close();
}

TEST_F(FSFixture, GetOutputExistingTest) {
    PriorityFS priority_fs{"prism_buffer"};
    {
        auto out_stream = std::ofstream{(buffer_path_ / fs::path{"file"}).native()};
        out_stream << "hello world";
    }
    std::ofstream stream;
    ASSERT_TRUE(fs::exists(buffer_path_ / fs::path{"file"}));
    EXPECT_FALSE(priority_fs.GetOutput("file", stream));
    EXPECT_FALSE(stream.is_open());
}

TEST_F(FSFixture, GetOutputWriteTest) {
    PriorityFS priority_fs{"prism_buffer"};
    std::ofstream stream;
    ASSERT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
    ASSERT_TRUE(priority_fs.GetOutput("file", stream));
    ASSERT_TRUE(stream.is_open());
    stream << "hello world";
    stream.close();

    auto in_stream = std::ifstream{(buffer_path_ / fs::path{"file"}).native()};
    std::string read((std::istreambuf_iterator<char>(in_stream)), std::istreambuf_iterator<char>());
    EXPECT_EQ(std::string{"hello world"}, read);
}

TEST_F(FSFixture, DeleteFalseTest) {
    PriorityFS priority_fs{"prism_buffer"};
    ASSERT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
    EXPECT_FALSE(priority_fs.Delete("file"));
    EXPECT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
}

TEST_F(FSFixture, DeleteTrueTest) {
    PriorityFS priority_fs{"prism_buffer"};
    {
        auto out_stream = std::ofstream{(buffer_path_ / fs::path{"file"}).native()};
        out_stream << "hello world";
    }
    ASSERT_TRUE(fs::exists(buffer_path_ / fs::path{"file"}));
    EXPECT_TRUE(priority_fs.Delete("file"));
    EXPECT_FALSE(fs::exists(buffer_path_ / fs::path{"file"}));
}
