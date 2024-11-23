//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#include <gtest/gtest.h> 

#include "libacpp-json/log.h"

#include <libacpp-json/parser.h>

using namespace libacpp::json;


TEST(ParserTests, Utf)
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

TEST(ParserTests, String)
{
struct Test {
    std::string json;
    ParseResult parseResult;
    std::string result;
}tests[]{
    {R"("hello")", ParseResult::ok, "hello"}, 
    {R"("hello)", ParseResult::partial, "hello"}, 
    {R"("hello\tworld!")", ParseResult::ok, "hello\tworld!"}, 
    {R"("hello\rworld!")", ParseResult::ok, "hello\rworld!"}, 
    {R"("hello\nworld!")", ParseResult::ok, "hello\nworld!"}, 
    {R"("hello\\world!")", ParseResult::ok, "hello\\world!"}, 
    {R"("hello world! \u00f1")", ParseResult::ok, "hello world! ñ"}, 
    {R"("\u0041 \u0042 \u0043 \u0044")", ParseResult::ok, "A B C D"}, 
};
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
    for(const auto& t: tests) {
        std::string result;
        StringParser sp(result);
        const char* p = t.json.data();
        const char* end = p + t.json.size();
        ParseResult r;
        while (p != end) {
            r = sp.parse(p, p+1);
        }
        EXPECT_EQ(r, t.parseResult) << "test 1";
        EXPECT_EQ(result, t.result) << "test 2";
        JSON_LOG_DEBUG("json: {}", t.json);
        JSON_LOG_DEBUG("result: {}", t.result);
    }
}


TEST(ParserTests, Number)
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
    //TODO complete error cases??
    {"abc", ParseResult::error, 0,0,0}, 
};
    std::cout << "Hello!!"  << std::endl;
    for(const auto& t: tests) {
        JSON_LOG_DEBUG("json: {}", t.json);
        //JSON_LOG_DEBUG("result: {}", t.result);
        int result;
        int frac;
        int exp;
        NumberParser sp(result, frac, exp);
        const char* p = t.json.data();
        auto r = sp.parse(p, p+t.json.size());
        EXPECT_EQ(r, t.parseResult) << "test 1";
        EXPECT_EQ(result, t.result) << "test 2";
        EXPECT_EQ(frac, t.frac) << "test 3";
        EXPECT_EQ(exp, t.exp) << "test 4";
    }
}



TEST(ParserTests, Literal)
{
    struct Test {
        std::string value;
        std::string literal;
        ParseResult parseResult;
    } 
    tests[]{
        {"null", "null", ParseResult::ok}, 
        {"true", "true", ParseResult::ok}, 
        {"false", "false", ParseResult::ok}, 
        {"Null", "null", ParseResult::error}, 
        {"nUll", "null", ParseResult::error}, 
        {"n", "null", ParseResult::partial}, 
        {"nullxxx", "null", ParseResult::ok}, 
    };
    JSON_LOG_DEBUG("first test");
    for(const auto& t: tests) {
        std::string result;
        LiteralParser sp(t.literal);
        const char* p = t.value.data();
        auto r = sp.parse(p, p + t.value.size());
        EXPECT_EQ(r, t.parseResult) << "test1 " << t.value;
    }
    JSON_LOG_DEBUG("second test");
    for(const auto& t: tests) {
        std::string result;
        LiteralParser sp(t.literal);
        const char* p = t.value.data();
        const char* end = p + t.value.size();
        ParseResult r = ParseResult::partial;
        while (p != end && r == ParseResult::partial) {
            r = sp.parse(p, p+1);
        }
        EXPECT_EQ(r, t.parseResult) << "test1 " << t.value << " " << (int)r << " " <<  (int)t.parseResult;
    }
}



TEST(ParserTests, Value)
{
    struct Test {
        std::string value;
        ParseResult parseResult;
        std::string string;
        int integer;
        int frac;
        int exp;
    } 
    tests[]{
        {"null", ParseResult::ok}, 
        {"true", ParseResult::ok}, 
        {"false", ParseResult::ok}, 
        {"Null", ParseResult::error}, 
        {"nUll", ParseResult::error}, 
        {"n", ParseResult::partial}, 
        {"nullxxx", ParseResult::ok}, 
        {R"("abc")", ParseResult::ok}, 
        {R"("abc)", ParseResult::partial}, 
        {"nullxxx", ParseResult::ok}, 
    };
    JSON_LOG_DEBUG("first test");
    for(const auto& t: tests) {
        JSON_LOG_DEBUG("testing {}", t.value);
        std::string result;
        ValueParser sp;
        const char* p = t.value.data();
        auto r = sp.parse(p, p + t.value.size());
        EXPECT_EQ(r, t.parseResult) << "test 1 " << t.value << " " << (int)r << " " <<  (int)t.parseResult;
        auto v = sp.value();
        if (v)
            JSON_LOG_DEBUG("value: {}", v->to_string());
    }
    JSON_LOG_DEBUG("second test");
    int c =0;
    for(const auto& t: tests) {
        JSON_LOG_DEBUG("testing {}", t.value);
        ++c;
        std::string result;
        ValueParser sp;
        const char* p = t.value.data();
        const char* end = p + t.value.size();
        ParseResult r = ParseResult::partial;
        while (p != end && r == ParseResult::partial) {
            r = sp.parse(p, p+1);
        }
        EXPECT_EQ(r, t.parseResult) << "test 1 " << t.value << " " << (int)r << " " <<  (int)t.parseResult << " test: " << c; 
        auto v = sp.value();
        if (v)
            JSON_LOG_DEBUG("value: {}", v->to_string());
    }


    // for(const auto& t: tests) {
    //     std::string result;
    //     LiteralParser sp(t.literal);
    //     const char* p = t.value.data();
    //     const char* end = p + t.value.size();
    //     ParseResult r;
    //     while (p != end && r == ParseResult::partial) {
    //         r = sp.parse(p, p+1);
    //     }
    //     EXPECT_EQ(r, t.parseResult) << "test 1";
    // }

}
