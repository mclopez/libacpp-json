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
    {"99999 ", ParseResult::ok, 99999, 0, 0}, 
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
    {"1E- ", ParseResult::error, 1, 0, 0}, 
    {"123.45e67", ParseResult::partial, 123, 45, 67}, 
    {"123.45e6a7", ParseResult::ok, 123, 45, 6}, 
    {"123.45", ParseResult::partial, 123, 45, 0}, 
    {"123.45 ", ParseResult::ok, 123, 45, 0}, 
    {"123.45ea", ParseResult::error, 123, 45, 0}, // a letter after exp 'e'
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

class JsonVisitor: public JsonVisitorBase {
public:

void key(std::string& k) {
    //JSON_LOG_DEBUG("JsonVisitor::key {}", k);
    key_ = k;
}

void null_value() override {
    for (int i=0; i<indent_; i++)
        result_ << "\t";
    result_ << "(NULL)" << key_ << "\n";
}

void string_value(std::string& value) override {
    for (int i=0; i<indent_; i++)
        result_ << "\t";
    result_ << "(STR)" << key_ << "/" << value << "\n";
}
void bool_value(bool value)  override {
    for (int i=0; i<indent_; i++)
        result_ << "\t";
    result_ << "(BOL)" << key_ << "/" << value << "\n";
}
void numbrer_value(int integer, int frac, int exp)  override {
    for (int i=0; i<indent_; i++)
        result_ << "\t";
    result_ << "(NUM)" << key_ << "/" << integer << "/" << frac<< "/" << exp << "\n";
}

void begin_object()  override {
    for (int i=0; i<indent_; i++)
        result_ << "\t";
    result_ << "(OBJ)" << key_ << '\n';
    ++ indent_;
}

void end_object()  override {
    -- indent_;
}

void begin_array()  override {
    for (int i=0; i<indent_; i++)
        result_ << "\t";
    result_ << "(ARR)" << key_<< '\n';
    key_.clear();    
    ++ indent_;
}

void end_array()  override {
    -- indent_;
}



std::string to_string() { return result_.str();} 

private:
    std::string key_;
    std::stringstream result_;
    int indent_ = 0;
};


TEST(ParserTests, Value)
{
    struct Test {
        std::string input;
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
        {"12345 ", ParseResult::ok}, 
        {"Null", ParseResult::error}, 
        {"nUll", ParseResult::error}, 
    //    {"n", ParseResult::partial}, 
        {"nullxxx", ParseResult::ok}, 
        {R"("abc")", ParseResult::ok}, 
        {R"("abc)", ParseResult::partial}, 
        {"nullxxx", ParseResult::ok}, 
    };
    JSON_LOG_DEBUG("FIRST test");
    for(const auto& t: tests) {
        JSON_LOG_DEBUG("testing input: {} **********************", t.input);
        std::string result;
        JsonVisitor jv;
        ObjectParser op(jv);
        ValueParser sp(jv);
        const char* p = t.input.data();
        auto r = sp.parse(p, p + t.input.size());
        EXPECT_EQ(r, t.parseResult) << "test 1 " << t.input << " " << (int)r << " " <<  (int)t.parseResult;

        JSON_LOG_DEBUG("jv: {}", jv.to_string());

    }
    JSON_LOG_DEBUG("SECOND test");
    int c =0;
    for(const auto& t: tests) {
        JSON_LOG_DEBUG("testing input: {}", t.input);
        ++c;
        std::string result;
        JsonVisitor jv;
        ObjectParser op(jv);
        ValueParser sp(jv);
        const char* p = t.input.data();
        const char* end = p + t.input.size();
        ParseResult r = ParseResult::partial;
        while (p != end && r == ParseResult::partial) {
            r = sp.parse(p, p+1);
        }

        JSON_LOG_DEBUG("jv: {}", jv.to_string());
        EXPECT_EQ(r, t.parseResult) << "test 1 " << t.input << " " << (int)r << " " <<  (int)t.parseResult << " test: " << c; 
        auto v = sp.value();
        if (v)
            JSON_LOG_DEBUG("value: {}", v->to_string());
    }


}




TEST(ParserTests, KeyValue)
{
    struct Test {
        std::string input;
        ParseResult parseResult;
        std::string result;
    } 
    tests[]{
        {R"("key2":x )", ParseResult::error, ""}, 
        {R"("key2":12.345eaa7 )", ParseResult::error, ""}, 
        {R"("key2":Null )", ParseResult::error, ""}, 
        {R"("key2":False )", ParseResult::error, ""}, 
        {R"("key2":truE )", ParseResult::error, ""}, 

        {R"("key1":"value1")", ParseResult::ok, "(STR)key1/value1\n"}, 
        {R"("key2":12345 )", ParseResult::ok, "(NUM)key2/12345/0/0\n"}, 
        {R"("key2":12.345e67 )", ParseResult::ok, "(NUM)key2/12/345/67\n"}, 
        {R"("key2":12.345e6a7 )", ParseResult::ok, "(NUM)key2/12/345/6\n"}, 
        {R"("key3":true )",  ParseResult::ok, "(BOL)key3/1\n"}, 
        {R"("key4":false )", ParseResult::ok, "(BOL)key4/0\n"}, 
        {R"("key5":null )",  ParseResult::ok, "(NULL)key5\n"}, 


    };
    //if (false)
    {
    JSON_LOG_DEBUG("first test");
    for(const auto& t: tests) {
        JSON_LOG_DEBUG("testing {}", t.input);
        JsonVisitor v;
        ObjectParser op(v);
        KeyValueParser sp(v);
        const char* p = t.input.data();
        auto r = sp.parse(p, p + t.input.size());

        EXPECT_EQ(r, t.parseResult);
        JSON_LOG_DEBUG("result {}", v.to_string());
        EXPECT_EQ(v.to_string(), t.result);
        
    }
    }
    JSON_LOG_DEBUG("second test");
    int c =0;
    for(const auto& t: tests) {
        JSON_LOG_DEBUG("testing {}", t.input);
        ++c;
        JsonVisitor v;
        ObjectParser op(v);
        KeyValueParser sp(v);
        const char* p = t.input.data();
        const char* end = p + t.input.size();
        ParseResult r = ParseResult::partial;
        while (p != end && r == ParseResult::partial) {
            r = sp.parse(p, p+1);
        }
        EXPECT_EQ(r, t.parseResult);
        JSON_LOG_DEBUG("result {}", v.to_string());
        EXPECT_EQ(v.to_string(), t.result);
   }

}


TEST(ParserTests, Object)
{
    struct Test {
        std::string input;
        ParseResult parseResult;
        std::string result;
    } 
    tests[]{
        {R"({null})", ParseResult::error, "(OBJ)\n"},  //TODO: this fails
        {R"({"key1":null})", ParseResult::ok, "(OBJ)\n\t(NULL)key1\n"}, 
        {R"({ "key1" : null })", ParseResult::ok, "(OBJ)\n\t(NULL)key1\n"}, 
        {R"({   "key1"  :   null    })", ParseResult::ok, "(OBJ)\n\t(NULL)key1\n"}, 
        {R"({ "key1" : null , "key2" : true })", ParseResult::ok, 
            "(OBJ)\n" 
            "\t(NULL)key1\n"
            "\t(BOL)key2/1\n"
        }, 

        {R"({"key1":{"key2":2}})", ParseResult::ok, 
                "(OBJ)\n" 
                "\t(OBJ)key1\n"
                "\t\t(NUM)key2/2/0/0\n"
        }, 
        {R"({"key1":{"key2":"abcd", "key3":1.234,"key4":{"key5": null, "key6": false}}})", ParseResult::ok, 
                "(OBJ)\n" 
                "\t(OBJ)key1\n"
                "\t\t(STR)key2/abcd\n"
                "\t\t(NUM)key3/1/234/0\n"
                "\t\t(OBJ)key4\n"
                "\t\t\t(NULL)key5\n"
                "\t\t\t(BOL)key6/0\n"
        }, 
        {R"({"key1":{"key2":1}})", ParseResult::ok, 
                "(OBJ)\n" 
                "\t(OBJ)key1\n"
                "\t\t(NUM)key2/1/0/0\n"
        }, 


    };
    if (true)
    {
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            JsonVisitor v;
            ObjectParser sp(v);
            const char* p = t.input.data();
            auto r = sp.parse(p, p + t.input.size());

            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result {}", v.to_string());
            EXPECT_EQ(v.to_string(), t.result);
            
        }
    }
    if (true)
    {
        JSON_LOG_DEBUG("second test");
        int c =0;
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            ++c;
            JsonVisitor v;
            ObjectParser op(v);
            const char* p = t.input.data();
            const char* end = p + t.input.size();
            ParseResult r = ParseResult::partial;
            while (p != end && r == ParseResult::partial) {
                r = op.parse(p, p+1);
            }
            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result {}", v.to_string());
            EXPECT_EQ(v.to_string(), t.result);
        }
    }

}


TEST(ParserTests, Array)
{
    struct Test {
        std::string input;
        ParseResult parseResult;
        std::string result;
    } 
    tests[]{
        {R"([null])", ParseResult::ok, "(ARR)\n\t(NULL)\n"}, 
        {R"([null, 123.45e-67, true, false, "abc"])", ParseResult::ok, 
            "(ARR)\n"
            "\t(NULL)\n"
            "\t(NUM)/123/45/-67\n"
            "\t(BOL)/1\n"
            "\t(BOL)/0\n"
            "\t(STR)/abc\n"
        }, 
    };
    if (true)
    {
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            JsonVisitor v;
            ArrayParser sp(v);
            const char* p = t.input.data();
            auto r = sp.parse(p, p + t.input.size());

            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result \n{}", v.to_string());
            EXPECT_EQ(v.to_string(), t.result);
            
        }
    }
    if (true)
    {
        JSON_LOG_DEBUG("second test");
        int c =0;
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            ++c;
            JsonVisitor v;
            ArrayParser op(v);
            const char* p = t.input.data();
            const char* end = p + t.input.size();
            ParseResult r = ParseResult::partial;
            while (p != end && r == ParseResult::partial) {
                r = op.parse(p, p+1);
            }
            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result \n{}", v.to_string());
            EXPECT_EQ(v.to_string(), t.result);
        }
    }

}

TEST(ParserTests, Full)
{
    struct Test {
        std::string input;
        ParseResult parseResult;
        std::string result;
    } 
    tests[]{
        {R"({null})", ParseResult::error, "(OBJ)\n"},  

        {R"({"key1":[null]})", ParseResult::ok, 
            "(OBJ)\n"
            "\t(ARR)key1\n"
            "\t\t(NULL)\n"
        },
        {R"({ "key1" : [null] } )", ParseResult::ok, 
            "(OBJ)\n"
            "\t(ARR)key1\n"
            "\t\t(NULL)\n"
        },

        {R"({"key1":[null, 123.45e-67, true, false, "abc"],"key2":{"key3": "abc", "key4": { } } } )", ParseResult::ok, 
            "(OBJ)\n"
            "\t(ARR)key1\n"
            "\t\t(NULL)\n"
            "\t\t(NUM)/123/45/-67\n"
            "\t\t(BOL)/1\n"
            "\t\t(BOL)/0\n"
            "\t\t(STR)/abc\n"
            "\t(OBJ)key2\n"
            "\t\t(STR)key3/abc\n"
            "\t\t(OBJ)key4\n"
        }, 

        {R"({ })", ParseResult::ok, "(OBJ)\n"},  


    };
    if (true)
    {
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            JsonVisitor v;
            ObjectParser sp(v);
            const char* p = t.input.data();
            auto r = sp.parse(p, p + t.input.size());

            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result \n{}", v.to_string());
            EXPECT_EQ(v.to_string(), t.result);
            
        }
    }
    if (false)
    {
        JSON_LOG_DEBUG("second test");
        int c =0;
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            ++c;
            JsonVisitor v;
            ObjectParser op(v);
            const char* p = t.input.data();
            const char* end = p + t.input.size();
            ParseResult r = ParseResult::partial;
            while (p != end && r == ParseResult::partial) {
                r = op.parse(p, p+1);
            }
            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result \n{}", v.to_string());
            EXPECT_EQ(v.to_string(), t.result);
        }
    }

}