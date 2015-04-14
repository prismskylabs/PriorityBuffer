#ifndef PRIORITY_BUFFER_H
#define PRIORITY_BUFFER_H

#include <algorithm>
#include <chrono>
#include <deque>
#include <functional>
#include <sstream>
#include <string>
#include <tuple>


template <typename T>
class PriorityBuffer {
    typedef std::function<unsigned long long(const T&)> PriorityFunction;
    typedef std::tuple<unsigned long long, T, std::string> PriorityObject;

  public:
    PriorityBuffer(PriorityFunction make_priority=&PriorityBuffer::epoch_priority_)
            : make_priority_{make_priority} {
        srand(std::chrono::steady_clock::now().time_since_epoch().count());
    }

    void Push(const T& t) {
        auto priority = make_priority_(t);
        auto find = std::find_if(objects_.begin(), objects_.end(),
                [&priority] (const PriorityObject& o) {
                    return priority >= get_priority_(o);
                });
        if (find == objects_.begin()) {
            objects_.emplace_front(priority, t, make_hash_());
        } else if (find == objects_.end()) {
            objects_.emplace_back(priority, t, make_hash_());
        } else {
            auto position = find - objects_.begin();
            std::rotate(objects_.begin(), objects_.begin() + position, objects_.end());
            objects_.emplace_front(priority, t, make_hash_());
            std::rotate(objects_.begin(), objects_.end() - position, objects_.end());
        }
    }

    T Pop() {
        auto object = objects_.front();
        objects_.pop_front();
        return get_object_(object);
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

    static std::string get_hash_(PriorityObject priority_object) {
        return std::get<2>(priority_object);
    }

    static T get_object_(PriorityObject priority_object) {
        return std::get<1>(priority_object);
    }

    static unsigned long long get_priority_(PriorityObject priority_object) {
        return std::get<0>(priority_object);
    }

    PriorityFunction make_priority_;
    std::deque<PriorityObject> objects_;
};

#endif
