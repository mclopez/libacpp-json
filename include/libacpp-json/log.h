//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#pragma once


#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h" 


#define INIT_LOGGER(name, pattern) spdlog::set_pattern("[%H:%M:%S] [%^%t%$] [%^%l%$] %v"); \
                                   auto name = spdlog::stdout_color_mt(#name)


#define JSON_LOG_TRACE(...)    spdlog::trace(__VA_ARGS__)
#define JSON_LOG_DEBUG(...)    spdlog::debug(__VA_ARGS__)
#define JSON_LOG_INFO(...)     spdlog::info(__VA_ARGS__)
#define JSON_LOG_WARN(...)     spdlog::warn(__VA_ARGS__)
#define JSON_LOG_ERROR(...)    spdlog::error(__VA_ARGS__)
#define JSON_LOG_CRITICAL(...) spdlog::critical(__VA_ARGS__)
