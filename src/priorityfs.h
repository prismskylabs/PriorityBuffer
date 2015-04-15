#ifndef PRIORITY_FS_H
#define PRIORITY_FS_H

#include <fstream>
#include <memory>
#include <string>


class PriorityFS {
  public:
    PriorityFS(const std::string& buffer_directory);
    ~PriorityFS();

    std::string GetFilePath(const std::string& file);
    std::ifstream GetInput(const std::string& file);
    std::ofstream GetOutput(const std::string& file);
    void Delete(const std::string& file);

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

#endif
