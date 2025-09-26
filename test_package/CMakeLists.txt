cmake_minimum_required(VERSION 3.15)
project(PackageTest CXX)

# Match the package name from conanfile.py
find_package(acpp-json CONFIG REQUIRED)

add_executable(test_package test_package.cpp)
target_link_libraries(test_package PRIVATE acpp-json::acppJson)


# Add verbose message to debug include directories
get_target_property(INCLUDE_DIRS acpp-json::acppJson INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "Include directories: ${INCLUDE_DIRS}")

# Ensure the executable can find the headers
target_include_directories(test_package PRIVATE ${INCLUDE_DIRS})