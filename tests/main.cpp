
/*
#include <iostream>

#include <libacpp-json/parser.h>


int main(){
    std::cout << "..." << std::endl;
}

*/
#include <gtest/gtest.h> // googletest header file

#include "libacpp-json/log.h"

#include <libacpp-json/parser.h>


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    INIT_LOGGER(logger, "[%H:%M:%S] [%^%t%$] [%^%l%$] %v"); // Initialize logger with custom pattern

    spdlog::set_level(spdlog::level::debug); 

    return RUN_ALL_TESTS();
}