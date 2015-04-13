#ifndef PRIORITY_BUFFER_H
#define PRIORITY_BUFFER_H

#include <algorithm>
#include <chrono>
#include <functional>
#include <deque>


template <typename T>
class PriorityBuffer {
    typedef std::function<unsigned long long(const T&)> PriorityFunction;
    typedef std::pair<unsigned long long, T> PriorityObject;

  public:
    PriorityBuffer(PriorityFunction get_priority=&PriorityBuffer::epoch_priority)
        : get_priority_{get_priority} {}

    void Push(const T& t) {
        auto priority = get_priority_(t);
        auto find = std::find_if(objects_.begin(), objects_.end(),
                [priority] (const PriorityObject& o) {
                    return priority >= o.first;
                });
        if (find == objects_.begin()) {
            objects_.emplace_front(priority, t);
        } else if (find == objects_.end()) {
            objects_.emplace_back(priority, t);
        } else {
            auto position = find - objects_.begin();
            std::rotate(objects_.begin(), objects_.begin() + position, objects_.end());
            objects_.emplace_front(priority, t);
            std::rotate(objects_.begin(), objects_.end() - position, objects_.end());
        }
    }

    T Pop() {
        auto object = objects_.front();
        objects_.pop_front();
        return object.second;
    }

  private:
    static unsigned long long epoch_priority(const T& t) {
        return std::chrono::steady_clock::now().time_since_epoch().count();
    }

    PriorityFunction get_priority_;
    std::deque<PriorityObject> objects_;
};

#endif
