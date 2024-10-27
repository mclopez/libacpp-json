enable_testing()

add_executable(acppJsonTests
    main.cpp
    parser_test.cpp
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


