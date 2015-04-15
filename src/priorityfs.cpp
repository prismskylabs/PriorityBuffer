#include "priorityfs.h"

#include <boost/filesystem.hpp>

#include <fstream>
#include <string>


namespace fs = boost::filesystem;

class PriorityFS::Impl {
  public:
    Impl(const std::string& buffer_directory);

    std::string GetFilePath(const std::string& file);
    bool GetInput(const std::string& file, std::ifstream& stream);
    bool GetOutput(const std::string& file, std::ofstream& stream);
    bool Delete(const std::string& file);

  private:
    fs::path buffer_path_;
};

PriorityFS::Impl::Impl(const std::string& buffer_directory) {
    buffer_path_ = fs::temp_directory_path() / fs::path{buffer_directory};
    fs::create_directory(buffer_path_);
}

std::string PriorityFS::Impl::GetFilePath(const std::string& file) {
    return (buffer_path_ / fs::path{file}).native();
}

bool PriorityFS::Impl::GetInput(const std::string& file, std::ifstream& stream) {
    auto file_path = buffer_path_ / fs::path{file};
    if (fs::exists(file_path)) {
        stream.open(file_path.native());
        return true;
    }
    return false;
}

bool PriorityFS::Impl::GetOutput(const std::string& file, std::ofstream& stream) {
    auto file_path = buffer_path_ / fs::path{file};
    if (!fs::exists(file_path)) {
        stream.open(file_path.native());
        return true;
    }
    return false;
}

bool PriorityFS::Impl::Delete(const std::string& file) {
    auto file_path = buffer_path_ / fs::path{file};
    return fs::remove(buffer_path_ / fs::path{file});
}


// Bridge

PriorityFS::PriorityFS(const std::string& buffer_directory)
        : pimpl_{ new Impl{buffer_directory} } {}
PriorityFS::~PriorityFS() {}

std::string PriorityFS::GetFilePath(const std::string& file) {
    return pimpl_->GetFilePath(file);
}

bool PriorityFS::GetInput(const std::string& file, std::ifstream& stream) {
    return pimpl_->GetInput(file, stream);
}

bool PriorityFS::GetOutput(const std::string& file, std::ofstream& stream) {
    return pimpl_->GetOutput(file, stream);
}

bool PriorityFS::Delete(const std::string& file) {
    return pimpl_->Delete(file);
}
