#include <algorithm>

#include <boost/filesystem.hpp>
#include <gtest/gtest.h>


namespace fs = ::boost::filesystem;

class FSFixture : public ::testing::Test {
  protected:
    virtual void SetUp() {
        buffer_path_ = fs::temp_directory_path() / fs::path{"prism_buffer"};
        fs::remove_all(buffer_path_);
    }

    virtual void TearDown() {
        fs::remove_all(buffer_path_);
    }

    int number_of_files_() {
        fs::directory_iterator begin(buffer_path_), end;
        return std::count_if(begin, end,
                [] (const fs::directory_entry& f) {
                    return !(fs::is_directory(f.path()) ||
                             f.path().filename().native().substr(0, 10) == "prism_data");
                });
    }

    fs::path buffer_path_;
};
