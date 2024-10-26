
add_library(
    acppJson
    parser.cpp
)

target_include_directories(acppJson PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

