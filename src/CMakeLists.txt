set(PRIORITYBUFFER_LIBRARIES prioritybuffer CACHE INTERNAL "Main object library for PriorityBuffer")
set(PRIORITYBUFFER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL
    "Main object library for PriorityBuffer")

include_directories(${PRIORITYBUFFER_INCLUDE_DIRS})
add_library(${PRIORITYBUFFER_LIBRARIES}
    prioritybuffer.h prioritybuffer.cpp)
