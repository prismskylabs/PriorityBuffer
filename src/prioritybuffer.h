#ifndef PRIORITY_BUFFER_H
#define PRIORITY_BUFFER_H

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>

#include "prioritydb.h"
#include "priorityfs.h"

#define DEFAULT_MAX_BUFFER_SIZE 100000000LL
#define DEFAULT_MAX_MEMORY_SIZE 50


template <typename T>
class PriorityBuffer {
    typedef std::function<unsigned long long(const T&)> PriorityFunction;

  public:
    PriorityBuffer()
            : make_priority_{epoch_priority_}, fs_{"prism_buffer", std::string{}},
              db_{DEFAULT_MAX_BUFFER_SIZE, fs_.GetFilePath("prism_data.db")},
              max_memory_{DEFAULT_MAX_MEMORY_SIZE}, fuzzer_{0, 0} {
        srand(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    PriorityBuffer(PriorityFunction make_priority)
            : make_priority_{make_priority}, fs_{"prism_buffer", std::string{}},
              db_{DEFAULT_MAX_BUFFER_SIZE, fs_.GetFilePath("prism_data.db")},
              max_memory_{DEFAULT_MAX_MEMORY_SIZE}, fuzzer_{0, 0} {
        srand(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    PriorityBuffer(PriorityFunction make_priority, const unsigned long long& buffer_size,
                   const int& max_memory)
            : make_priority_{make_priority}, fs_{"prism_buffer", std::string{}},
              db_{buffer_size, fs_.GetFilePath("prism_data.db")}, max_memory_{max_memory}, 
              fuzzer_{0, 0} {
        srand(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    PriorityBuffer(PriorityFunction make_priority, const std::string& buffer_root,
                   const unsigned long long& buffer_size, const int& max_memory)
            : make_priority_{make_priority}, fs_{"prism_buffer", std::string{buffer_root}},
              db_{buffer_size, fs_.GetFilePath("prism_data.db")}, max_memory_{max_memory}, 
              fuzzer_{0, 0} {
        srand(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    ~PriorityBuffer() {
        for (auto object = objects_.begin(); object != objects_.end(); ++object) {
            auto hash = object->first;
            save_to_disk(object->second, hash);
        }
    }

    void SetFuzz(const unsigned long& fuzz_lower_ms, const unsigned long& fuzz_upper_ms) {
        std::lock_guard<std::mutex> lock(mutex_);
        fuzzer_ = std::uniform_int_distribution<unsigned long>{fuzz_lower_ms, fuzz_upper_ms};
    }

    void Push(T& t) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto hash = make_hash_();
        auto size = get_size_(t);
        db_.Insert(make_priority_(t), hash, size);
        objects_.emplace(hash, std::move(t));

        while (objects_.size() > max_memory_) {
            auto lowest_hash = db_.GetLowestMemoryHash();
            auto find = objects_.find(lowest_hash);
            if (find != objects_.end()) {
                auto object = find->second;
                save_to_disk(object, lowest_hash);
                objects_.erase(lowest_hash);
            }
        }

        while (db_.Full()) {
            auto lowest_hash = db_.GetLowestDiskHash();
            fs_.Delete(lowest_hash);
            db_.Delete(lowest_hash);
        }

        condition_.notify_one();;
    }

    T Pop(bool block=false)
    {
        T object;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            bool on_disk = true;
            auto hash = db_.GetHighestHash(on_disk);
            if (block) {
                while (hash.empty()) {
                    condition_.wait(lock);
                    hash = db_.GetHighestHash(on_disk);
                }
            }

            db_.Delete(hash);

            if (!on_disk) {
                auto find = objects_.find(hash);
                if (find != objects_.end()) {
                    object = find->second;
                    objects_.erase(hash);
                }
            } else {
                object = inflate(hash);
            }
        }

        if (object.IsInitialized() && fuzzer_.b() > 0 && fuzzer_.a() <= fuzzer_.b()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(fuzzer_(generator_)));
        }

        return object;
    }

  protected:
    PriorityFS fs_;
    PriorityDB db_;
    std::map<std::string, T> objects_;
    std::mutex mutex_;
    std::condition_variable condition_;

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
        std::ifstream file_stream;
        T t;
        if (fs_.GetInput(hash, file_stream) && file_stream.is_open())
        {
            t.ParseFromIstream(&file_stream);
            t.CheckInitialized();
            file_stream.close();
            fs_.Delete(hash);
        }
        return t;
        ;
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

    PriorityFunction make_priority_;
    int max_memory_;
    std::random_device generator_;
    std::uniform_int_distribution<unsigned long> fuzzer_;
};

#endif
