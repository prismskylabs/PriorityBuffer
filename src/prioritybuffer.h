#ifndef PRIORITY_BUFFER_H
#define PRIORITY_BUFFER_H

#include <vector>


template <typename T>
class PriorityBuffer {
  public:
    void Push(const T& t) {
        objects.push_back(t);
    }

    T& Pop() {
        auto& object = objects.back();
        objects.pop_back();
        return object;
    }

  private:
    std::vector<T> objects;
};

#endif
