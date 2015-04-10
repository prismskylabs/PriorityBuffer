#include <iostream>

#include "prioritybuffer.h"


int main() {
    auto buffer = PriorityBuffer<int>{};
    buffer.Push(5);
    buffer.Push(6);
    std::cout << "Item: " << buffer.Pop() << std::endl;
    std::cout << "Item: " << buffer.Pop() << std::endl;
    return 0;
}
