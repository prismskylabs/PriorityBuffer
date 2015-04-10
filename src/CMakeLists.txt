find_package(Protobuf REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${PROTOBUF_INCLUDE_DIRS})

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS addressbook.proto)

add_executable(pbmain pbmain.cpp ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(pbmain
    ${PROTOBUF_LIBRARIES})

add_executable(main main.cpp)
