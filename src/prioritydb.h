#ifndef PRIORITY_DB_H
#define PRIORITY_DB_H

#include <memory>
#include <string>


class PriorityDB {
  public:
    PriorityDB(const unsigned long long& max_size);
    ~PriorityDB();

    int Open(const std::string& path);
    void Insert(const unsigned long long& priority, const std::string& hash,
                const unsigned long long& size, const bool& on_disk=false);
    void Delete(const std::string& hash);
    void Update(const std::string& hash, const bool& on_disk);
    std::string GetHighestHash(bool& on_disk);
    std::string GetLowestMemoryHash();
    std::string GetLowestDiskHash();
    bool Full();

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

#endif
