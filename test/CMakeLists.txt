if (${NUMBER_MESSAGES_IN_TEST} EQUAL 50 OR ${NUMBER_MESSAGES_IN_TEST} GREATER 50)
    add_definitions(-DNUMBER_MESSAGES_IN_TEST=${NUMBER_MESSAGES_IN_TEST})
else()
    add_definitions(-DNUMBER_MESSAGES_IN_TEST=1000)
endif()

PROTOBUF_GENERATE_CPP(BASIC_PROTO_SRCS BASIC_PROTO_HDRS basic.proto)

add_executable(basic_pb_tests
    basic_pb_tests.cpp
    bufferfixture.h
    ${BASIC_PROTO_SRCS} ${BASIC_PROTO_HDRS})

target_include_directories(basic_pb_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${BOOSTFILESYSTEM_INCLUDE_DIRS})

target_link_libraries(basic_pb_tests
    ${GTEST_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES}
    ${PROTOBUF_LIBRARIES})

add_test(NAME basic_pb_tests COMMAND basic_pb_tests)

PROTOBUF_GENERATE_CPP(PRIORITY_PROTO_SRCS PRIORITY_PROTO_HDRS priority.proto)

add_executable(priority_pb_tests
    bufferfixture.h
    priority_pb_tests.cpp
    ${PRIORITY_PROTO_SRCS} ${PRIORITY_PROTO_HDRS})

target_include_directories(priority_pb_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${BOOSTFILESYSTEM_INCLUDE_DIRS})

target_link_libraries(priority_pb_tests
    ${GTEST_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES}
    ${PROTOBUF_LIBRARIES})

add_test(NAME priority_pb_tests COMMAND priority_pb_tests)

add_executable(failure_recovery_tests
    failure_recovery_tests.cpp
    bufferfixture.h
    ${PRIORITY_PROTO_SRCS} ${PRIORITY_PROTO_HDRS})

target_include_directories(failure_recovery_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${BOOSTFILESYSTEM_INCLUDE_DIRS})

target_link_libraries(failure_recovery_tests
    ${GTEST_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES}
    ${PROTOBUF_LIBRARIES})

add_test(NAME failure_recovery_tests COMMAND failure_recovery_tests)

add_executable(multithread_tests
    multithread_tests.cpp
    bufferfixture.h
    ${PRIORITY_PROTO_SRCS} ${PRIORITY_PROTO_HDRS})

target_include_directories(multithread_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    ${BOOSTFILESYSTEM_INCLUDE_DIRS})

target_link_libraries(multithread_tests
    ${GTEST_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES}
    ${PROTOBUF_LIBRARIES})

add_test(NAME multithread_tests COMMAND multithread_tests)

add_executable(fs_tests
    fs_tests.cpp)

target_include_directories(fs_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${BOOSTFILESYSTEM_INCLUDE_DIRS})

target_link_libraries(fs_tests
    ${GTEST_MAIN_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES})

add_test(NAME fs_tests COMMAND fs_tests)

add_executable(db_tests
    db_tests.cpp)

target_include_directories(db_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${BOOSTFILESYSTEM_INCLUDE_DIRS})

target_link_libraries(db_tests
    ${GTEST_MAIN_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES})

add_test(NAME db_tests COMMAND db_tests)
