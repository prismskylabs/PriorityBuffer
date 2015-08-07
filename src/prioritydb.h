#ifndef PRIORITYBUFFER_DB_H
#define PRIORITYBUFFER_DB_H

#include <memory>
#include <string>


namespace prism {
namespace prioritybuffer {

class PriorityDB {
  public:
    PriorityDB(const unsigned long long& max_size, const std::string& path);
    ~PriorityDB();

    void Insert(const unsigned long long& priority, const std::string& hash,
                const unsigned long long& size, const bool& on_disk=false);
    void Delete(const std::string& hash);
    void Update(const std::string& hash, const bool& on_disk);
    std::string GetHighestHash(bool& on_disk);
    std::string GetLowestMemoryHash();
    std::string GetLowestDiskHash();
    bool Full();
    int GetDiskLength();
    unsigned long long GetDiskSize();

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

class PriorityDBException : public std::exception {
  public:
    PriorityDBException(const std::string& reason) : reason_(reason) {}
    virtual const char* what() const throw() {
        return reason_.data();
    }

  private:
    std::string reason_;
};

} // namespace prioritybuffer
} // namespace prism

#endif /* PRIORITYBUFFER_DB_H */
