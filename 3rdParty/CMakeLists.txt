macro(_set_cache VAR_NAME VAR_DESCRIPTION)
    set(${VAR_NAME} ${${VAR_NAME}} CACHE INTERNAL ${VAR_DESCRIPTION})
    message(STATUS "|  ${VAR_NAME}: ${${VAR_NAME}}")
endmacro()

message(STATUS "/================================================================================")
message(STATUS "| 3rdParty Configuration")
message(STATUS "|================================================================================")

# Code coverage

if(BUILD_PRIORITYBUFFER_TESTS AND GENERATE_COVERAGE)
    set(CMAKE_BUILD_TYPE COVERAGE CACHE INTERNAL "Coverage enabled build")
    if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        set(COVERAGE_FLAG "--coverage")
    endif()
    set(CMAKE_CXX_FLAGS_COVERAGE
        "-g -O0 ${COVERAGE_FLAG} -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C++ compiler during coverage builds."
        FORCE)
    set(CMAKE_C_FLAGS_COVERAGE
        "-g -O0 ${COVERAGE_FLAG} -fprofile-arcs -ftest-coverage"
        CACHE STRING "Flags used by the C compiler during coverage builds."
        FORCE)
    set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
        ""
        CACHE STRING "Flags used for linking binaries during coverage builds."
        FORCE)
    set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
        ""
        CACHE STRING "Flags used by the shared libraries linker during coverage builds."
        FORCE)
    mark_as_advanced(
        CMAKE_CXX_FLAGS_COVERAGE
        CMAKE_C_FLAGS_COVERAGE
        CMAKE_EXE_LINKER_FLAGS_COVERAGE
        CMAKE_SHARED_LINKER_FLAGS_COVERAGE)
endif()


# GTest configuration

if(BUILD_PRIORITYBUFFER_TESTS)
    message(STATUS "| googletest Configuration")
    message(STATUS "|================================================================================")
    if(USE_SYSTEM_GTEST)
        find_package(GTest)
    endif()
    if(NOT USE_SYSTEM_GTEST AND NOT GTEST_FOUND)
        add_subdirectory(googletest)
        set(GTEST_FOUND TRUE)
        cmake_policy(SET CMP0026 OLD)
        get_property(GTEST_LIBRARIES TARGET gtest PROPERTY LOCATION)
        get_property(GTEST_MAIN_LIBRARIES TARGET gtest_main PROPERTY LOCATION)
        set(GTEST_BOTH_LIBRARIES ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})
        set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include)
    endif()

    _set_cache(GTEST_FOUND "Set if libgtest was found or built.")
    _set_cache(GTEST_LIBRARIES "Location of libgtest.")
    _set_cache(GTEST_MAIN_LIBRARIES "Location of libgtest-main.")
    _set_cache(GTEST_BOTH_LIBRARIES "Location of both gtest libraries.")
    _set_cache(GTEST_INCLUDE_DIRS "Location of gtest include files.")
    message(STATUS "|================================================================================")
endif()


# Protobuf configuration

if(BUILD_PRIORITYBUFFER_TESTS)
    message(STATUS "| Protobuf Configuration")
    message(STATUS "|================================================================================")

    find_package(Protobuf REQUIRED)

    if(USE_SYSTEM_PROTOBUF)
        find_package(Protobuf)
    endif()
    if(NOT USE_SYSTEM_PROTOBUF OR NOT PROTOBUF_FOUND)
        message(FATAL_ERROR "System libprotobuf and protoc is required to proceed.\n"
            "If you're using OS X and have Homebrew installed, you can use the command:\n"
            "\tbrew install protobuf\n"
            "If you're using a Linux distribution like Ubuntu, you can use the command:\n"
            "\tapt-get install libprotobuf-dev protoc")
    endif()

    _set_cache(PROTOBUF_FOUND "Set if libprotobuf was found or built.")
    _set_cache(PROTOBUF_LIBRARIES "Location of libprotobuf.")
    _set_cache(PROTOBUF_INCLUDE_DIRS "Location of protobuf include files.")
    message(STATUS "|================================================================================")
endif()


# sqlite3

message(STATUS "| sqlite3 Configuration")
message(STATUS "|================================================================================")

add_subdirectory(sqlite3)

_set_cache(SQLITE_FOUND "Set if libsqlite3 was found or built.")
_set_cache(SQLITE_LIBRARIES "Location of libsqlite3.")
_set_cache(SQLITE_INCLUDE_DIRS "Location of sqlite3 include files.")
message(STATUS "|================================================================================")


# Boost Filesystem

message(STATUS "| Boost Filesystem Configuration")
message(STATUS "|================================================================================")

if (USE_SYSTEM_BOOST)
    find_package(Boost COMPONENTS system filesystem REQUIRED)
    set(BOOSTSYSTEM_FOUND TRUE)
    set(BOOSTSYSTEM_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    set(BOOSTSYSTEM_LIBRARIES ${Boost_LIBRARIES})
    set(BOOSTFILESYSTEM_FOUND TRUE)
    set(BOOSTFILESYSTEM_INCLUDE_DIRS ${Boost_INCLUDE_DIRS})
    set(BOOSTFILESYSTEM_LIBRARIES ${Boost_LIBRARIES})
else()
    set(BOOST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/boostinclude)
    add_subdirectory(boostsystem)
    add_subdirectory(boostfilesystem)
endif()

_set_cache(BOOSTSYSTEM_FOUND "Set if libboostsystem was found or built.")
_set_cache(BOOSTSYSTEM_LIBRARIES "Location of libboostsystem.")
_set_cache(BOOSTSYSTEM_INCLUDE_DIRS "Location of boost system include files.")
_set_cache(BOOSTFILESYSTEM_FOUND "Set if libboostfilesystem was found or built.")
_set_cache(BOOSTFILESYSTEM_LIBRARIES "Location of libboostfilesystem.")
_set_cache(BOOSTFILESYSTEM_INCLUDE_DIRS "Location of boost filesystem include files.")
message(STATUS "|================================================================================")
