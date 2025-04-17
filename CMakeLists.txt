cmake_minimum_required(VERSION 3.15)
project(libacpp-json)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Conan integration
#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()


include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)


find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)


add_subdirectory(src)
add_subdirectory(tests)