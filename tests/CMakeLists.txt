enable_testing()

cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(acppJsonTests
    main.cpp
    parser_test.cpp
    reflection_test.cpp
)

target_include_directories(acppJsonTests 
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)


target_link_libraries(acppJsonTests PRIVATE acppJson)

target_link_libraries(acppJsonTests 
PRIVATE
    gtest::gtest
    spdlog::spdlog
)

# Add tests
add_test(NAME libacppJsonTests COMMAND libacpp-json)


