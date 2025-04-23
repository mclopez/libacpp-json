
add_library(
    acppJson
    utils.cpp
    parser.cpp
    consumer.cpp
)

target_include_directories(acppJson PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(acppJson 
PRIVATE
    spdlog::spdlog
)
