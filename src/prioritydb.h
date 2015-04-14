#ifndef PRIORITY_DB_H
#define PRIORITY_DB_H

#include <memory>
#include <string>


class PriorityDB {
  public:
    PriorityDB(const std::string& path=":memory:");
    ~PriorityDB();

    void Insert(const unsigned long long& priority, const std::string& hash,
                const bool& on_disk=false);
    void Delete(const std::string& hash);
    std::string GetHighestHash();

  private:
    class Impl;
    std::unique_ptr<Impl> pimpl_;
};

#endif
