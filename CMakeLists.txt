cmake_minimum_required(VERSION 3.15)
project(acpp-json VERSION 0.1.0)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)
find_package(spdlog REQUIRED)


add_subdirectory(src)


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_subdirectory(tests)
else()
    message(WARNING "Tests directory 'tests' not found - skipping tests")
endif()



install(TARGETS acppJson DESTINATION lib)

install(DIRECTORY include/libacpp-json/
        DESTINATION include/libacpp-json/
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.inl"
        PATTERN "*.hpp")