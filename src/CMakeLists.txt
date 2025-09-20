set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_OSX_SYSROOT ${MACOSX_SDK_PATH})

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

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

    execute_process(
        COMMAND xcodebuild -version -sdk macosx Path
        OUTPUT_VARIABLE MACOSX_SDK_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(CMAKE_OSX_SYSROOT ${MACOSX_SDK_PATH})

endif()
