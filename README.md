# PriorityBuffer [![Build Status](https://travis-ci.org/prismskylabs/PriorityBuffer.svg?branch=master)](https://travis-ci.org/prismskylabs/PriorityBuffer) [![Coverage Status](https://coveralls.io/repos/prismskylabs/PriorityBuffer/badge.svg)](https://coveralls.io/r/prismskylabs/PriorityBuffer)

PriorityBuffer is an intelligent cache for your protobuf messages. Built to be robust, threadsafe, and flexible, it simplifies the task of buffering your messages by priority, keeping the lowest priority messages stored on disk until they're needed again:

```protobuf
// basic.proto
message Basic {
  required string value = 1;
}
```

```c++
#include <prioritybuffer.h>
#include <iostream>
#include <memory>
#include "basic.pb.h"

int main(int argc,  char** argv) {
    prism::prioritybuffer::PriorityBuffer<Basic> buffer;
    for (int i = 0; i < 1000; ++i) {
        auto basic = std::unique_ptr<Basic>{ new Basic{} };
        basic.set_value("hello world!");
        buffer.Push(std::move(basic));
    }
    for (int i = 0; i < 1000; ++i) {
        auto basic = buffer.Pop();
        std::cout << basic.value() << std::endl; // Prints hello world!
    }
}
```

Underneath the hood, the vast majority of these messages were buffered to disk according to their priority.

By default, the priority is just the epoch time of the `Push` call. You can use a custom priority heuristic very easily:

```c++
#include <prioritybuffer.h>
#include <iostream>
#include <memory>
#include "basic.pb.h"

int priority_function(const Basic& basic) {
    return basic.value().length();
}

int main(int argc,  char** argv) {
    prism::prioritybuffer::PriorityBuffer<Basic> buffer{priority_function};
    for (int i = 0; i < 1000; ++i) {
        auto basic = std::unique_ptr<Basic>{ new Basic{} };
        basic.set_value(std::to_string(i));
        buffer.Push(std::move(basic));
    }
    for (int i = 0; i < 1000; ++i) {
        auto basic = buffer.Pop();
        std::cout << basic.value() << std::endl; // Prints 1000 first,
                                                 // then three digit numbers,
                                                 // then two,
                                                 // then one
    }
}
```

## Requirements

* A C++11 compatible compiler such as a suitably recent version of [clang](http://clang.llvm.org/) or [gcc](https://gcc.gnu.org/)
* [CMake](https://github.com/Kitware/CMake)
* [Google protobuf](https://github.com/google/protobuf)

## Build

The recommended way to build this library is by using an out of source build directory:

```shell
mkdir build
cd build
cmake ..
make
```

By default, this will build the tests. To run the tests, simply run:

```
ctest
```

This project has embedded sources of the required boost libraries and googletest framework so they are not explicitly required on your system. To use your system versions, set these CMake options:

```
cmake -DUSE_SYSTEM_BOOST=ON ..
```

or

```
cmake -DUSE_SYSTEM_GTEST=ON ..
```

and then run `make`.

A successful build will result in a single library that you can link against your project.

## Contributing

Please fork this repository and contribute back using [pull requests](https://github.com/prismskylabs/PriorityBuffer/pulls). Features can be requested using [issues](https://github.com/prismskylabs/PriorityBuffer/issues).
