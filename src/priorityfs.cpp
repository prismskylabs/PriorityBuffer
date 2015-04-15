#include "priorityfs.h"

#include <boost/filesystem.hpp>

#include <fstream>
#include <string>


namespace fs = boost::filesystem;

class PriorityFS::Impl {
  public:
    Impl(const std::string& buffer_directory);

    std::string GetFilePath(const std::string& file);
    std::ifstream GetInput(const std::string& file);
    std::ofstream GetOutput(const std::string& file);
    void Delete(const std::string& file);

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

std::ifstream PriorityFS::Impl::GetInput(const std::string& file) {
    auto file_path = buffer_path_ / fs::path{file};
    std::ifstream stream;
    if (fs::exists(file_path)) {
        stream.open(file_path.native());
    }
    return stream;
}

std::ofstream PriorityFS::Impl::GetOutput(const std::string& file) {
    auto file_path = buffer_path_ / fs::path{file};
    std::ofstream stream;
    if (!fs::exists(file_path)) {
        stream.open(file_path.native());
    }
    return stream;
}

void PriorityFS::Impl::Delete(const std::string& file) {
    auto file_path = buffer_path_ / fs::path{file};
    fs::remove(buffer_path_ / fs::path{file});
}


// Bridge

PriorityFS::PriorityFS(const std::string& buffer_directory)
        : pimpl_{ new Impl{buffer_directory} } {}
PriorityFS::~PriorityFS() {}

std::string PriorityFS::GetFilePath(const std::string& file) {
    return pimpl_->GetFilePath(file);
}

std::ifstream PriorityFS::GetInput(const std::string& file) {
    return pimpl_->GetInput(file);
}

std::ofstream PriorityFS::GetOutput(const std::string& file) {
    return pimpl_->GetOutput(file);
}

void PriorityFS::Delete(const std::string& file) {
    pimpl_->Delete(file);
}
