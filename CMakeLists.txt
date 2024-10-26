cmake_minimum_required(VERSION 3.15)
project(libacpp-json)

#include(ExternalProject)

set(CMAKE_CXX_STANDARD 17)

# Conan integration
#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
#conan_basic_setup()

add_subdirectory(src)
add_subdirectory(tests)