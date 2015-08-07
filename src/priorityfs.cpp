#include "priorityfs.h"

#include <exception>

#include <boost/filesystem.hpp>

#include <fstream>
#include <string>


namespace fs = boost::filesystem;
namespace prism {
namespace prioritybuffer {

class PriorityFS::Impl {
  public:
    Impl(const std::string& buffer_directory, const std::string& buffer_parent);

    std::string GetFilePath(const std::string& file);
    bool GetInput(const std::string& file, std::ifstream& stream);
    bool GetOutput(const std::string& file, std::ofstream& stream);
    bool Delete(const std::string& file);

  private:
    fs::path buffer_path_;
};

PriorityFS::Impl::Impl(const std::string& buffer_directory, const std::string& buffer_parent) {
    auto parent_path = buffer_parent.empty() ? fs::temp_directory_path() : fs::path{buffer_parent};
    if (buffer_directory.empty()) {
        throw PriorityFSException{"Cannot initialize PriorityFS with an empty buffer path"};
    }
    buffer_path_ = parent_path / fs::path{buffer_directory};
    if (fs::equivalent(buffer_path_, parent_path) ||
            fs::equivalent(buffer_path_, parent_path / fs::path{".."})) {
        throw PriorityFSException{"PriorityFS must be initialized within a valid parent directory"};
    }
    fs::create_directory(buffer_path_);
}

std::string PriorityFS::Impl::GetFilePath(const std::string& file) {
    return (buffer_path_ / fs::path{file}).string();
}

bool PriorityFS::Impl::GetInput(const std::string& file, std::ifstream& stream) {
    auto file_path = buffer_path_ / fs::path{file};
    if (!fs::is_directory(file_path) &&
            std::string{".."} != file_path.filename().string() &&
            fs::exists(file_path)) {
        stream.open(file_path.native());
        return true;
    }
    return false;
}

bool PriorityFS::Impl::GetOutput(const std::string& file, std::ofstream& stream) {
    auto file_path = buffer_path_ / fs::path{file};
    if (!fs::is_directory(file_path) &&
            std::string{".."} != file_path.filename().string() &&
            !fs::exists(file_path)) {
        stream.open(file_path.native());
        return true;
    }
    return false;
}

bool PriorityFS::Impl::Delete(const std::string& file) {
    auto file_path = buffer_path_ / fs::path{file};
    if (!fs::is_directory(file_path) &&
            std::string{".."} != file_path.filename().string() &&
            fs::exists(file_path)) {
        return fs::remove(buffer_path_ / fs::path{file});
    }
    return false;
}


// Bridge

PriorityFS::PriorityFS(const std::string& buffer_directory, const std::string& buffer_parent)
        : pimpl_{ new Impl{buffer_directory, buffer_parent} } {}
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

} // namespace prioritybuffer
} // namespace prism
