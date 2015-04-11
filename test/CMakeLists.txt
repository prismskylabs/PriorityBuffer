include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PRIORITYBUFFER_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS})

PROTOBUF_GENERATE_CPP(BASIC_PROTO_SRCS BASIC_PROTO_HDRS basic.proto)

add_executable(basic_pb_tests
    basic_pb_tests.cpp
    ${BASIC_PROTO_SRCS} ${BASIC_PROTO_HDRS})

target_link_libraries(basic_pb_tests
    ${GTEST_LIBRARIES}
    ${PRIORITYBUFFER_LIBRARIES}
    ${PROTOBUF_LIBRARIES})

add_test(NAME basic_pb_tests COMMAND basic_pb_tests)
