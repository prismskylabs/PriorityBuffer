#include <boost/filesystem.hpp>
#include <gtest/gtest.h>


namespace fs = boost::filesystem;

class BufferFixture : public ::testing::Test {
  protected:
    virtual void SetUp() {
        fs::remove_all(fs::temp_directory_path() / fs::path{"prism_buffer"});
    }

    virtual void TearDown() {
        fs::remove_all(fs::temp_directory_path() / fs::path{"prism_buffer"});
    }
};
