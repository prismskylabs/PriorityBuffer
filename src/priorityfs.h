#ifndef PRIORITYBUFFER_FS_H
#define PRIORITYBUFFER_FS_H

#include <fstream>
#include <memory>
#include <string>


namespace prism {
namespace prioritybuffer {

class PriorityFS {
  public:
    PriorityFS(const std::string& buffer_directory, const std::string& buffer_parent=std::string{});
    ~PriorityFS();

    std::string GetFilePath(const std::string& file);
    bool GetInput(const std::string& file, std::ifstream& stream);
    bool GetOutput(const std::string& file, std::ofstream& stream);
    bool Delete(const std::string& file);

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

class PriorityFSException : public std::exception {
  public:
    PriorityFSException(const std::string& reason) : reason_(reason) {}
    virtual const char* what() const throw() {
        return reason_.data();
    }

  private:
    std::string reason_;
};

} // namespace prioritybuffer
} // namespace prism

#endif /* PRIORITYBUFFER_FS_H */
