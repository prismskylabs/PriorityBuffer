macro(_set_cache VAR_NAME VAR_DESCRIPTION)
    set(${VAR_NAME} ${${VAR_NAME}} CACHE INTERNAL ${VAR_DESCRIPTION})
    message(STATUS "Set ${VAR_NAME} to ${${VAR_NAME}}.")
endmacro()


# GTest configuration

if(BUILD_PRIORITYBUFFER_TESTS)
    if(USE_SYSTEM_GTEST)
        find_package(GTest)
    endif()
    if(NOT USE_SYSTEM_GTEST OR NOT GTEST_FOUND)
        message(STATUS "Not using system gtest, using built-in googletest project instead.")
        add_subdirectory(googletest)
        set(GTEST_FOUND TRUE)
        set(GTEST_LIBRARIES gtest)
        set(GTEST_MAIN_LIBRARIES gtest_main)
        set(GTEST_BOTH_LIBRARIES gtest gtest_main)
        set(GTEST_INCLUDE_DIRS ${gtest_SOURCE_DIR}/include)
    endif()

    _set_cache(GTEST_FOUND "Set if libgtest was found or built")
    _set_cache(GTEST_LIBRARIES "Location of libgtest")
    _set_cache(GTEST_MAIN_LIBRARIES "Location of libgtest-main")
    _set_cache(GTEST_BOTH_LIBRARIES "Location of both gtest libraries")
    _set_cache(GTEST_INCLUDE_DIRS "Location of gtest include files")
endif()


# Protobuf configuration

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

_set_cache(PROTOBUF_FOUND "Set if libprotobuf was found or built")
_set_cache(PROTOBUF_LIBRARIES "Location of libprotobuf")
_set_cache(PROTOBUF_INCLUDE_DIRS "Location of protobuf include files")
