set(PRIORITYBUFFER_LIBRARIES prioritybuffer CACHE INTERNAL "Main object library for PriorityBuffer")
set(PRIORITYBUFFER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL
    "Main object library for PriorityBuffer")

include_directories(
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${SQLITE_INCLUDE_DIRS}
    ${BOOSTFILESYSTEM_INCLUDE_DIRS})

add_library(${PRIORITYBUFFER_LIBRARIES}
    prioritybuffer.h prioritybuffer.cpp
    prioritydb.h prioritydb.cpp
    priorityfs.h priorityfs.cpp)

target_link_libraries(${PRIORITYBUFFER_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${BOOSTFILESYSTEM_LIBRARIES})
