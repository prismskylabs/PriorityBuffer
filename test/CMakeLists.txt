PROTOBUF_GENERATE_CPP(BASIC_PROTO_SRCS BASIC_PROTO_HDRS basic.proto)

add_executable(basic_pb_tests
    basic_pb_tests.cpp
    ${BASIC_PROTO_SRCS} ${BASIC_PROTO_HDRS})

target_include_directories(basic_pb_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS})

target_link_libraries(basic_pb_tests
    ${GTEST_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES}
    ${PROTOBUF_LIBRARIES})

add_test(NAME basic_pb_tests COMMAND basic_pb_tests)

PROTOBUF_GENERATE_CPP(PRIORITY_PROTO_SRCS PRIORITY_PROTO_HDRS priority.proto)

add_executable(priority_pb_tests
    priority_pb_tests.cpp
    ${PRIORITY_PROTO_SRCS} ${PRIORITY_PROTO_HDRS})

target_include_directories(priority_pb_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS})

target_link_libraries(priority_pb_tests
    ${GTEST_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES}
    ${PROTOBUF_LIBRARIES})

add_test(NAME priority_pb_tests COMMAND priority_pb_tests)
