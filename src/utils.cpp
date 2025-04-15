//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)



#include <stdexcept>
	
#include <libacpp-json/utils.h>

namespace libacpp::json {

bool is_hex(uint8_t c, uint8_t& r) {
    if ('0' <= c  && c <= '9') {
        r = c - '0';
        return true;
    }
    if ('a' <= c && c <= 'f') {
        r = ((uint8_t)10) + (c - 'a');
        return true;
    }
    if ('A' <= c && c <= 'F') {
        r = ((uint8_t)10) + (c - 'A');
        return true;
    }
    return false;
}

std::vector<uint8_t> unicode_to_utf8(uint32_t codepoint) {
    std::vector<uint8_t> utf8;

    if (codepoint <= 0x7F) {
        // 1-byte sequence: 0xxxxxxx
        utf8.push_back(static_cast<uint8_t>(codepoint));
    } else if (codepoint <= 0x7FF) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        utf8.push_back(static_cast<uint8_t>((codepoint >> 6) | 0xC0));
        utf8.push_back(static_cast<uint8_t>((codepoint & 0x3F) | 0x80));
    } else if (codepoint <= 0xFFFF) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        utf8.push_back(static_cast<uint8_t>((codepoint >> 12) | 0xE0));
        utf8.push_back(static_cast<uint8_t>(((codepoint >> 6) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>((codepoint & 0x3F) | 0x80));
    } else if (codepoint <= 0x10FFFF) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        utf8.push_back(static_cast<uint8_t>((codepoint >> 18) | 0xF0));
        utf8.push_back(static_cast<uint8_t>(((codepoint >> 12) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>(((codepoint >> 6) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>((codepoint & 0x3F) | 0x80));
    } else {
        // Invalid code point
        throw std::invalid_argument("Invalid Unicode code point");
    }

    return utf8;
}


} // namespace libacpp::json