#ifndef PRIORITY_BUFFER_H
#define PRIORITY_BUFFER_H

#include <algorithm>
#include <chrono>
#include <functional>
#include <map>
#include <sstream>
#include <string>

#include "prioritydb.h"


template <typename T>
class PriorityBuffer {
    typedef std::function<unsigned long long(const T&)> PriorityFunction;

  public:
    PriorityBuffer(PriorityFunction make_priority=&PriorityBuffer::epoch_priority_)
            : make_priority_{make_priority}, db_{} {
        srand(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    void Push(const T& t) {
        auto hash = make_hash_();
        objects_[hash] = t;
        db_.Insert(make_priority_(t), hash);
    }

    T Pop() {
        auto hash = db_.GetHighestHash();
        auto object = objects_[hash];
        objects_.erase(hash);
        db_.Delete(hash);
        return object;
    }

  private:
    static unsigned long long epoch_priority_(const T& t) {
        return std::chrono::steady_clock::now().time_since_epoch().count();
    }

    static std::string make_hash_(const int& len=32) {
        static const char alphanum[] = "0123456789"
                                       "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                       "abcdefghijklmnopqrstuvwxyz";

        std::stringstream stream;
        for (int i = 0; i < len; ++i) {
            stream << alphanum[rand() % (sizeof(alphanum) - 1)];
        }

        return stream.str();
    }

    PriorityDB db_;
    PriorityFunction make_priority_;
    std::map<std::string, T> objects_;
};

#endif
