#ifndef PRIORITY_BUFFER_H
#define PRIORITY_BUFFER_H

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <sstream>
#include <string>

#include "prioritydb.h"
#include "priorityfs.h"


template <typename T>
class PriorityBuffer {
    typedef std::function<unsigned long long(const T&)> PriorityFunction;

  public:
    PriorityBuffer(PriorityFunction make_priority=&PriorityBuffer::epoch_priority_,
                   const unsigned long long& max_size=100000000LL, const int& max_memory=50,
                   const std::string& buffer_directory="prism_buffer")
                : make_priority_{make_priority}, fs_{buffer_directory},
                  db_{max_size, fs_.GetFilePath("prism_data.db")},
                  max_memory_{max_memory} {
        srand(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    ~PriorityBuffer() {
        for (auto object = objects_.begin(); object != objects_.end(); ++object) {
            auto hash = object->first;
            save_to_disk(object->second, hash);
        }
    }

    void Push(const T& t) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto hash = make_hash_();
        objects_[hash] = t;
        auto size = get_size_(t);
        db_.Insert(make_priority_(t), hash, size);

        if (objects_.size() > max_memory_) {
            auto lowest_hash = db_.GetLowestMemoryHash();
            auto object = objects_[lowest_hash];
            objects_.erase(lowest_hash);
            save_to_disk(object, lowest_hash);
        }

        while (db_.Full()) {
            auto lowest_hash = db_.GetLowestDiskHash();
            fs_.Delete(lowest_hash);
            db_.Delete(lowest_hash);
        }
    }

    T Pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        bool on_disk;
        auto hash = db_.GetHighestHash(on_disk);

        if (!on_disk) {
            auto object = objects_[hash];
            objects_.erase(hash);
            db_.Delete(hash);
            return object;
        } else {
            auto object = inflate(hash);
            db_.Delete(hash);
            return object;
        }
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

    static unsigned long get_size_(const T& t) {
        return t.ByteSize();
    }

    T inflate(const std::string& hash) {
        T t;
        std::ifstream file_stream;
        if (fs_.GetInput(hash, file_stream) && file_stream.is_open()) {
            t.ParseFromIstream(&file_stream);
            t.CheckInitialized();
            file_stream.close();
            fs_.Delete(hash);
        }
        return t;
    }

    bool save_to_disk(const T& t, const std::string& hash) {
        std::ofstream file_stream;
        if (fs_.GetOutput(hash, file_stream) && file_stream.is_open()) {
            t.SerializeToOstream(&file_stream);
            file_stream.close();
            db_.Update(hash, true);
            return true;
        }
        fs_.Delete(hash);
        db_.Delete(hash);
        return false;
    }

    PriorityFS fs_;
    PriorityDB db_;
    PriorityFunction make_priority_;
    std::map<std::string, T> objects_;
    std::mutex mutex_;
    int max_memory_;
};

#endif
