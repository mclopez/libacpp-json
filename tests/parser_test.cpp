//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#include <gtest/gtest.h> 

#include "libacpp-json/log.h"

#include <libacpp-json/parser.h>

using namespace libacpp::json;


TEST(ParserTests, utf)
{
    struct Test {
    uint32_t codepoint;
    std::vector<uint8_t> result;
} 
tests[]{
    //U+00F1	ñ	c3 b1
    'a', {'a'},
    0x00f1, {0xc3, 0xb1},
    0x1B00, {0xe1, 0xac, 0x80},
    0x1fa00, {0xf0, 0x9f, 0xa8, 0x80}
};
    //std::vector<uint8_t> val = unicode_to_tf8() ;
    for(const auto& t: tests) {
        auto r = unicode_to_tf8(t.codepoint);
        EXPECT_EQ(t.result, r);
    }
}

TEST(ParserTests, string)
{
struct Test {
    std::string json;
    ParseResult parseResult;
    std::string result;
} 
tests[]{
    {R"("hello")", ParseResult::ok, "hello"}, 
    {R"("hello\tworld!")", ParseResult::ok, "hello\tworld!"}, 
    {R"("hello\rworld!")", ParseResult::ok, "hello\rworld!"}, 
    {R"("hello\nworld!")", ParseResult::ok, "hello\nworld!"}, 
    {R"("hello\\world!")", ParseResult::ok, "hello\\world!"}, 
    {R"("hello world! \u00f1")", ParseResult::ok, "hello world! ñ"}, 
    {R"("\u0041 \u0042 \u0043 \u0044")", ParseResult::ok, "A B C D"}, 
};
    std::cout << "Hello!!"  << std::endl;
    for(const auto& t: tests) {
        std::string result;
        StringParser sp(result);
        const char* p = t.json.data();
        auto r = sp.parse(p, p+t.json.size());
        EXPECT_EQ(r, t.parseResult) << "test 1";
        EXPECT_EQ(result, t.result) << "test 2";
        JSON_LOG_DEBUG("json: {}", t.json);
        JSON_LOG_DEBUG("result: {}", t.result);
    }
}


TEST(ParserTests, integer)
{
struct Test {
    std::string json;
    ParseResult parseResult;
    int result;
    int frac;
    int exp;
} 
tests[]{
    {"0", ParseResult::ok, 0, 0, 0}, 
    {"01", ParseResult::ok, 0, 0, 0}, 
    {"1", ParseResult::partial, 1, 0, 0}, 
    {"5", ParseResult::partial, 5, 0, 0}, 
    {"9", ParseResult::partial, 9, 0, 0}, 
    {"10", ParseResult::partial, 10, 0, 0}, 
    {"15", ParseResult::partial, 15, 0, 0}, 
    {"99", ParseResult::partial, 99, 0, 0}, 
    {"100", ParseResult::partial, 100, 0, 0}, 
    {"11111", ParseResult::partial, 11111, 0, 0}, 
    {"99999", ParseResult::partial, 99999, 0, 0}, 
    {"-1", ParseResult::partial, -1, 0, 0}, 
    {"-11", ParseResult::partial, -11, 0, 0}, 
    {"-111", ParseResult::partial, -111, 0, 0}, 
    {"-9", ParseResult::partial, -9, 0, 0}, 
    {"-99", ParseResult::partial, -99, 0, 0}, 
    {"-999", ParseResult::partial, -999, 0, 0}, 
    {"1.1", ParseResult::partial, 1, 1, 0}, 
    {"123456789.123456789", ParseResult::partial, 123456789, 123456789, 0}, 
    {"1e1", ParseResult::partial, 1, 0, 1}, 
    {"1E1", ParseResult::partial, 1, 0, 1}, 
    {"1e+1", ParseResult::partial, 1, 0, 1}, 
    {"1E-1", ParseResult::partial, 1, 0, -1}, 
    {"123.45e67", ParseResult::partial, 123, 45, 67}, 
    {"-123.45e67", ParseResult::partial, -123, 45, 67}, 
    {"123.45e-67", ParseResult::partial, 123, 45, -67}, 
    {"-123.45e-67", ParseResult::partial, -123, 45, -67}, 
};
    std::cout << "Hello!!"  << std::endl;
    for(const auto& t: tests) {
        JSON_LOG_DEBUG("json: {}", t.json);
        //JSON_LOG_DEBUG("result: {}", t.result);
        int result;
        int frac;
        int exp;
        IntegerParser sp(result, frac, exp);
        const char* p = t.json.data();
        auto r = sp.parse(p, p+t.json.size());
        EXPECT_EQ(r, t.parseResult) << "test 1";
        EXPECT_EQ(result, t.result) << "test 2";
        EXPECT_EQ(frac, t.frac) << "test 3";
        EXPECT_EQ(exp, t.exp) << "test 4";
    }
}