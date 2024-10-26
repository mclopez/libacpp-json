enable_testing()

add_executable(acppJsonTests
    main.cpp
)

target_include_directories(acppJsonTests 
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)


target_link_libraries(acppJsonTests PRIVATE acppJson)

# Add tests
add_test(NAME libacppJsonTests COMMAND libacpp-json)