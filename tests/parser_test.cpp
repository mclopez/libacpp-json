//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#include <gtest/gtest.h> 

#include "libacpp-json/log.h"

#include <libacpp-json/parser.h>

using namespace libacpp::json;


class JsonConsumer {
public:
    // Begin String Consumer
    void string_begin() {
        string_value_.clear();    
    }
    void add_char_string(char c) { 
        string_value_.push_back(c);
    }
    void string_end() {
        std::cout << "string_value_: " << string_value_ << std::endl; 
        type_ = ValueType::string; 
        indent_value();
        ss_ << '"' << string_value_ << '"' << '\n';
    }
    // End String Consumer

    // Begin Key Consumer
    void key_begin() {
        key_.clear();    
    }
    void add_char_key(char c) { 
        key_.push_back(c);
    }
    void key_end() {
        std::cout << "string_: " << key_ << std::endl; 
        indent_key();
        ss_ << key_ << " -> ";
    }
    // End String Consumer


    std::string key() { return key_;}
    std::string string_value() { return string_value_;}

    // Number Consumer
    void number_begin() {
        int_ = 0;
        frac_ = 0;
        exp_ = 0;
        sign_ = 1;
    }


    void sign(int s) {
        sign_ = s;
    } 

    void add_char_int(char c) {
        int_ = (int_ * 10) + sign_ * (c - '0');
    }

    void add_char_frac(char c) {
        frac_ = (frac_ * 10) + (c - '0');
    }

    void add_char_exp(char c) {
        exp_ = (exp_ * 10) + sign_ * (c - '0');
    }

    void number_end() {
        type_ = ValueType::number; 
        indent_value();
        ss_ << "(NUM)/" << int_ << '/' << frac_ << '/' << exp_ << '\n'; 
    }

    // End Number Consumer


    int integer() {return int_;}
    int frac() {return frac_;}
    int exp() {return exp_;}



    void set_bool(bool v)  { 
        type_ = ValueType::boolean; 
        bool_value_ = v; 
        indent_value();
        ss_ << (v?"true":"false") << '\n'; 
    }

    void set_null() { 
        type_ = ValueType::nil;  
        indent_value();
        ss_ << "null\n";
    }


    ValueType type() {return type_;}


    typedef JsonConsumer KeyConsumerType;
    typedef JsonConsumer ValueConsumerType;
    KeyConsumerType& key_consumer() { return *this;}
    ValueConsumerType& value_consumer(){ return *this;}

    typedef JsonConsumer NumberConsumerType;
    typedef JsonConsumer StringConsumerType;
    typedef JsonConsumer ObjectConsumerType;
    typedef JsonConsumer ArrayConsumerType;
    NumberConsumerType& number_consumer() { return *this;}
    StringConsumerType& string_consumer() { return *this;}
    ObjectConsumerType& object_consumer() { return *this;}
    ArrayConsumerType& array_consumer() { return *this;}
    

    bool as_bool() { return bool_value_; }


    typedef JsonConsumer KeyValueConsumerType;

    //Begin Object Consumer
    void object_begin() {
        object_ = true;
        ss_ << "(OBJ)" << '\n';
        ++indent_;
        indent_value();
        type_ = ValueType::object; 
    }

    void object_end() {
        --indent_;
        object_ = false;
    }
    //End Object Consumer

    //Begin Array Consumer
    void array_begin() {
        array_ = true;
        ss_ << "(ARR)" << '\n';
        ++indent_;
        type_ = ValueType::array; 
    }

    void array_end() {
        --indent_;
        array_ = false;
        indent_value();
    }
    //End Array Consumer
    
    void indent_key() {
        std::cout << "indent_key" << std::endl;
        if (object_) 
            for(int i=0; i<indent_; ++i)  
                ss_ << '\t';
    }

    void indent_value() { 
        std::cout << "indent_value" << std::endl;
        if (array_)
            for(int i=0; i<indent_; ++i)  
                ss_ << '\t';
    }

    std::string to_string() { return ss_.str();}

private:
    int indent_ = 0;
    std::stringstream ss_;
    std::string key_;

    int int_ = 0;
    int frac_ = 0;
    int exp_ = 0;
    int sign_ = 1;

    ValueType type_;

    bool bool_value_;
    std::string string_value_;
    bool array_ = false;
    bool object_ = false;
};

bool sync_test = true;
bool async_test = true;


TEST(ParserTests2, String)
{
struct Test {
    std::string input;
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
    if (sync_test)
        for(const auto& t: tests) {
            JsonConsumer consumer;
            StringParser<JsonConsumer, false> sp{consumer};
            const char* p = t.input.data();
            auto r = sp.parse(p, p+t.input.size());
            EXPECT_EQ(r, t.parseResult) << "test 1";
            EXPECT_EQ(sp.consumer().string_value(), t.result) << "test 2";
            JSON_LOG_DEBUG("json: {}", t.input);
            JSON_LOG_DEBUG("result: {}", t.result);
        }
    if (async_test)
        for(const auto& t: tests) {
            JsonConsumer consumer;
            StringParser<JsonConsumer> sp{consumer};
            const char* p = t.input.data();
            const char* end = p + t.input.size();
            ParseResult r;
            while (p != end) {
                r = sp.parse(p, p+1);
            }
            EXPECT_EQ(r, t.parseResult) << "test 1";
            EXPECT_EQ(sp.consumer().string_value(), t.result) << "test 2";
            JSON_LOG_DEBUG("json: {}", t.input);
            JSON_LOG_DEBUG("result: {}", t.result);
        }
}


TEST(ParserTests2, Number)
{
struct Test {
    std::string input;
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
    if (sync_test) {
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("input: {}", t.input);
            JsonConsumer consumer;
            NumberParser<JsonConsumer> sp{consumer};
            const char* p = t.input.data();
            auto r = sp.parse(p, p+t.input.size());
            EXPECT_EQ(r, t.parseResult) << "test 1";
            EXPECT_EQ(sp.consumer().integer(), t.result) << "test 2";
            EXPECT_EQ(sp.consumer().frac(), t.frac) << "test 3";
            EXPECT_EQ(sp.consumer().exp(), t.exp) << "test 4";
        }
    }
    if (async_test) {
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("input: {}", t.input);
            JsonConsumer consumer;
            NumberParser<JsonConsumer> sp{consumer};

            ParseResult r = ParseResult::partial;
            for(char c: t.input) {
                char b[2];
                b[0] = c;
                b[1] = 0;
                const char* p = &b[0];
                const char* end = p+1;
                r = sp.parse(p, end);
                if (r != ParseResult::partial)
                    break;
            }


            EXPECT_EQ(r, t.parseResult) << "test 1";
            EXPECT_EQ(sp.consumer().integer(), t.result) << "test 2";
            EXPECT_EQ(sp.consumer().frac(), t.frac) << "test 3";
            EXPECT_EQ(sp.consumer().exp(), t.exp) << "test 4";
        }
    }
}


TEST(ParserTests2, Literal)
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
    if (sync_test) {
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            std::string result;
            LiteralParser sp(t.literal);
            const char* p = t.value.data();
            auto r = sp.parse(p, p + t.value.size());
            EXPECT_EQ(r, t.parseResult) << "test1 " << t.value;
        }
    }
    if (async_test) {
        JSON_LOG_DEBUG("second test");
        for(const auto& t: tests) {
            std::string result;
            LiteralParser sp(t.literal);
            const char* p = t.value.data();
            const char* end = p + t.value.size();
            ParseResult r = ParseResult::partial;
            for(char c: t.value) {
                char b[2];
                b[0] = c;
                b[1] = 0;
                const char* p = &b[0];
                const char* end = p+1;
                r = sp.parse(p, end);
                if (r != ParseResult::partial)
                    break;
            }

            EXPECT_EQ(r, t.parseResult) << "test1 " << t.value << " " << (int)r << " " <<  (int)t.parseResult;
        }
    }
}

template <typename ValueConsumer>
std::string to_string(ValueConsumer& vc) {
    std::stringstream ss;

    switch(vc.type()) {
        case ValueType::undef: 
            ss << "(UNDEF)";
            break;
        case ValueType::nil: 
            ss << "(NULL)";
            break;
        case ValueType::boolean: 
            ss << "(BOL)" << vc.as_bool();
            break;
        case ValueType::number: 
            ss << "(NUM)" << vc.number_consumer().integer() << "/" << vc.number_consumer().frac() << "/" << vc.number_consumer().exp();
            break;
        case ValueType::string: 
            ss << "(STR)" << vc.string_value();
            break;
        case ValueType::object: 
            ss << "(OBJ)" << "<to be compleated>";
            break;
        case ValueType::array: 
            ss << "(ARR)" << "<to be compleated>";
            break;
        default:
            break;    
    }
    return ss.str();
} 

TEST(ParserTests2, Value)
{
    struct Test {
        std::string input;
        ParseResult parseResult;
        std::string result;
    } 
    tests[]{
        {"null", ParseResult::ok, "(NULL)"}, 
        {"true", ParseResult::ok, "(BOL)1"}, 
        {"false", ParseResult::ok, "(BOL)0"}, 
        {"12345 ", ParseResult::ok, "(NUM)12345/0/0"}, 
        {"12345. ", ParseResult::error, ""}, 
        {"Null", ParseResult::error, ""}, 
        {"nUll", ParseResult::error, ""}, 
        {"n", ParseResult::partial, ""}, 
        {"nullxxx", ParseResult::ok, "(NULL)"}, 
        {R"("abc")", ParseResult::ok, "(STR)abc"}, 
        {R"("abc)", ParseResult::partial, ""}, 
    };

    if (sync_test) { 
        JSON_LOG_DEBUG("FIRST test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing input: {} **********************", t.input);
            JsonConsumer consumer;
            ValueParser<JsonConsumer> sp(consumer);
            const char* p = t.input.data();
            auto r = sp.parse(p, p + t.input.size());
            EXPECT_EQ(r, t.parseResult) << "test 1 " << t.input << " " << (int)r << " " <<  (int)t.parseResult;
            std::string result = to_string(consumer);
            JSON_LOG_DEBUG("result: {}", result);
            if (r == ParseResult::ok) {
                EXPECT_EQ(result, t.result) << "test 1 " << t.input << " " <<  t.result;            
            }
        }
    }
    if (async_test) { 
        JSON_LOG_DEBUG("SECOND test");
        int c =0;
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing input: {}", t.input);
            ++c;
            JsonConsumer consumer;
            ValueParser<JsonConsumer> sp(consumer);
            const char* p = t.input.data();
            const char* end = p + t.input.size();
            ParseResult r = ParseResult::partial;
            for(char c: t.input) {
                char b[2];
                b[0] = c;
                b[1] = 0;
                const char* p = &b[0];
                const char* end = p+1;
                r = sp.parse(p, end);
                if (r != ParseResult::partial)
                    break;
            }

            EXPECT_EQ(r, t.parseResult) << "test 1 " << t.input << " " << (int)r << " " <<  (int)t.parseResult << " test: " << c; 
            std::string result = to_string(consumer);
            JSON_LOG_DEBUG("result: {}", result);
            if (r == ParseResult::ok) {
                EXPECT_EQ(result, t.result) << "test 1 " << t.input << " " <<  t.result;            
            }
        }
    }

}

std::string key_value_to_string(JsonConsumer& c) {
    std::stringstream ss;
    ss << c.key() << "->" << to_string(c.value_consumer()) 
    << '\n';
    return ss.str();
}

TEST(ParserTests2, KeyValue)
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

        {R"("key1":"value1")", ParseResult::ok, "key1->(STR)value1\n"}, 
        {R"("key2":12345 )", ParseResult::ok, "key2->(NUM)12345/0/0\n"}, 
        {R"("key2":12.345e67 )", ParseResult::ok, "key2->(NUM)12/345/67\n"}, 
        {R"("key2":12.345e6a7 )", ParseResult::ok, "key2->(NUM)12/345/6\n"}, 
        {R"("key3":true )",  ParseResult::ok, "key3->(BOL)1\n"}, 
        {R"("key4":false )", ParseResult::ok, "key4->(BOL)0\n"}, 
        {R"("key5":null )",  ParseResult::ok, "key5->(NULL)\n"}, 
    };

    if (sync_test) { 
        JSON_LOG_DEBUG("first test");
        if (false)
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            JsonConsumer consumer;
            KeyValueParser<JsonConsumer> sp(consumer);
            const char* p = t.input.data();
            auto r = sp.parse(p, p + t.input.size());

            EXPECT_EQ(r, t.parseResult);
            if (r == ParseResult::ok) {
                std::string result = key_value_to_string(consumer);
                JSON_LOG_DEBUG("result {}", result);
                EXPECT_EQ(result, t.result);
            }
            
        }
    }
    if (async_test) { 
        JSON_LOG_DEBUG("second test");
        int c =0;
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            ++c;
            JsonConsumer consumer;
            KeyValueParser<JsonConsumer> sp(consumer);
            const char* p = t.input.data();
            const char* end = p + t.input.size();
            ParseResult r = ParseResult::partial;
            for(char c: t.input) {
                char b[2];
                b[0] = c;
                b[1] = 0;
                const char* p = &b[0];
                const char* end = p+1;
                r = sp.parse(p, end);
                if (r != ParseResult::partial)
                    break;
            }

            EXPECT_EQ(r, t.parseResult);
            if (r == ParseResult::ok) {
                std::string result = key_value_to_string(consumer);
                JSON_LOG_DEBUG("result {}", result);
                EXPECT_EQ(result, t.result);
            }
        }
    }
}



TEST(ParserTests2, Object)
{
    struct Test {
        std::string input;
        ParseResult parseResult;
        std::string result;
    } 
    tests[]{
        {R"({null})", ParseResult::error, "(OBJ)\n"},  
        {R"({"key1":null})", ParseResult::ok, "(OBJ)\n\tkey1 -> null\n"}, 
        {R"({ "key1" : null })", ParseResult::ok, "(OBJ)\n\tkey1 -> null\n"}, 
        {R"({   "key1"  :   null    })", ParseResult::ok, "(OBJ)\n\tkey1 -> null\n"}, 
        {R"({ "key1" : null , "key2" : true })", ParseResult::ok, 
            "(OBJ)\n" 
            "\tkey1 -> null\n"
            "\tkey2 -> true\n"
        }, 
        {R"({"key1":{"key2":2}})", ParseResult::ok, 
                "(OBJ)\n" 
                "\tkey1 -> (OBJ)\n"
                "\t\tkey2 -> (NUM)/2/0/0\n"
        }, 
        {R"({"key1":{"key2":"abcd", "key3":1.234,"key4":{"key5": null, "key6": false}}})", ParseResult::ok, 
                "(OBJ)\n" 
                "\tkey1 -> (OBJ)\n"
                "\t\tkey2 -> \"abcd\"\n"
                "\t\tkey3 -> (NUM)/1/234/0\n"
                "\t\tkey4 -> (OBJ)\n"
                "\t\t\tkey5 -> null\n"
                "\t\t\tkey6 -> false\n"
        }, 
        {R"({"key1":{"key2":1}})", ParseResult::ok, 
                "(OBJ)\n" 
                "\tkey1 -> (OBJ)\n"
                "\t\tkey2 -> (NUM)/1/0/0\n"
        }, 


    };
    if (sync_test) { 
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            JsonConsumer oc;
            ObjectParser<JsonConsumer> sp{oc};
            const char* p = t.input.data();
            auto r = sp.parse(p, p + t.input.size());

            EXPECT_EQ(r, t.parseResult);

            JSON_LOG_DEBUG("result {}", oc.to_string());
            EXPECT_EQ(oc.to_string(), t.result);
            
        }
    }
    if (async_test) { 
        JSON_LOG_DEBUG("second test");
        int c =0;
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            ++c;
            JsonConsumer oc;
            ObjectParser<JsonConsumer> op{oc};
            const char* p = t.input.data();
            const char* end = p + t.input.size();
            ParseResult r = ParseResult::partial;
            for(char c: t.input) {
                char b[2];
                b[0] = c;
                b[1] = 0;
                const char* p = &b[0];
                const char* end = p+1;
                r = op.parse(p, end);
                if (r != ParseResult::partial)
                    break;
            }
            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result {}", oc.to_string());
            EXPECT_EQ(oc.to_string(), t.result);
        }
    }
}



TEST(ParserTests2, Array)
{
    struct Test {
        std::string input;
        ParseResult parseResult;
        std::string result;
    } 
    tests[]{
        {R"([null])", ParseResult::ok, "(ARR)\n\tnull\n"}, 
        {R"([null, 123.45e-67, true, false, "abc"])", ParseResult::ok, 
            "(ARR)\n"
            "\tnull\n"
            "\t(NUM)/123/45/-67\n"
            "\ttrue\n"
            "\tfalse\n"
            "\t\"abc\"\n"
        }, 
    };
    if (sync_test) { 
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            JsonConsumer oc;
            ArrayParser<JsonConsumer> ap{oc};
            const char* p = t.input.data();
            auto r = ap.parse(p, p + t.input.size());

            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result \n{}", oc.to_string());
            EXPECT_EQ(oc.to_string(), t.result);
            
        }
    }
    if (async_test) { 
        JSON_LOG_DEBUG("second test");
        int c =0;
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            ++c;
            JsonConsumer oc;
            ArrayParser<JsonConsumer> ap{oc};
            const char* p = t.input.data();
            const char* end = p + t.input.size();
            ParseResult r = ParseResult::partial;
            for(char c: t.input) {
                char b[2];
                b[0] = c;
                b[1] = 0;
                const char* p = &b[0];
                const char* end = p+1;
                r = ap.parse(p, end);
                if (r != ParseResult::partial)
                    break;
            }

            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result \n{}", oc.to_string());
            EXPECT_EQ(oc.to_string(), t.result);
        }
    }

}



TEST(ParserTests2, Full)
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
            "\tkey1 -> (ARR)\n"
            "\t\tnull\n"
        },

        {R"({ "key1" : [null] } )", ParseResult::ok, 
            "(OBJ)\n"
            "\tkey1 -> (ARR)\n"
            "\t\tnull\n"
        },

        {R"({"key1":[null, 123.45e-67, true, false, "abc"],"key2":{"key3": "abcd", "key4": { } } } )", ParseResult::ok, 
            "(OBJ)\n"
            "\tkey1 -> (ARR)\n"
            "\t\tnull\n"
            "\t\t(NUM)/123/45/-67\n"
            "\t\ttrue\n"
            "\t\tfalse\n"
            "\t\t\"abc\"\n"
            "\tkey2 -> (OBJ)\n"
            "\t\tkey3 -> \"abcd\"\n"
            "\t\tkey4 -> (OBJ)\n"
        }, 

        {R"({ })", ParseResult::ok, "(OBJ)\n"},  


    };
    if (sync_test) { 
        JSON_LOG_DEBUG("first test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            JsonConsumer oc;
            ObjectParser<JsonConsumer> vp{oc};
            const char* p = t.input.data();
            auto r = vp.parse(p, p + t.input.size());

            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result \n{}", oc.to_string());
            EXPECT_EQ(oc.to_string(), t.result);
            
        }
    }
    if (async_test) { 
        JSON_LOG_DEBUG("second test");
        int c =0;
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing {}", t.input);
            ++c;
            JsonConsumer oc;
            ValueParser<JsonConsumer> vp{oc};
            ParseResult r = ParseResult::partial;
            for(char c: t.input) {
                char b[2];
                b[0] = c;
                b[1] = 0;
                const char* p = &b[0];
                const char* end = p+1;
                r = vp.parse(p, end);
                if (r != ParseResult::partial)
                    break;
            }
            EXPECT_EQ(r, t.parseResult);
            JSON_LOG_DEBUG("result \n{}", oc.to_string());
            EXPECT_EQ(oc.to_string(), t.result);
        }
    }

}