#include <functional>
#include <type_traits>

#include <gtest/gtest.h> 

#include "libacpp-json/log.h"
#include <libacpp-json/parser.h>
#include <libacpp-json/consumer.h>



//libacpp::json::Conf conf;
struct  X: public   std::variant<std::string, int, double, bool, std::nullptr_t> {
public:
typedef  std::variant<std::string, int, double, bool, std::nullptr_t> variant_type;

template< typename T>
void operator=(const T& s) {
    variant_type::operator=(s);
}   


};


TEST(ConsumerTests, test1) {
    using namespace libacpp::json;
    JsonValue v1(1);
    JsonValue v2("hola");
    // JsonValue v3; 
    // auto s  = std::string("hola");
    // v3 = s;
    std::variant<std::string, int, double, bool, std::nullptr_t> var;
    var = "hola";
    X x; 
    x = "hola";
    //JsonObject m{{"v1", std::make_unique<JsonValue>(1)}, {"hola", std::make_unique<JsonValue>(2)}};
//     JsonObject o;
// //    o["hola"] = std::make_unique<JsonValue>("hola");
//     o.get<JsonObject>().["hola"] = JsonValue("hola");
//     o["hola"] = JsonValue("hola2");
//     //std::map<std::string, JsonValue>{
    JsonValue o2{JsonObject()};
    std::get<JsonObject>(o2)["hola"] = JsonValue("hola");
    auto v = std::get<JsonObject>(o2)["hola"];
    std::cout << "get: " << std::get<std::string>(v) << std::endl;
    std::get<JsonObject>(o2)["hola"] = JsonValue{"mundo"};
    std::get<JsonObject>(o2)["hola2"] = JsonValue{"mundo2"};

    JsonValue a{JsonArray({JsonValue("HOLA"), JsonValue("MUNDO")})};
    std::get<JsonObject>(o2)["hola3"] = a;


    v = std::get<JsonObject>(o2)["hola"];    
    std::cout << "get: " << std::get<std::string>(v) << std::endl;
    std::stringstream ss;
    to_string(o2, ss);
    std::cout << "get: " << ss.str() << std::endl;

}


using namespace libacpp::json;

TEST(ConsumerTests, test2) {
    struct Test {
        std::string input;
        ParseResult parseResult;
        std::string result;
    } 
    tests[]{
        {"null", ParseResult::ok, "null"}, 
        {"true", ParseResult::ok, "true"}, 
        {"false", ParseResult::ok, "false"}, 
        {"12345 ", ParseResult::ok, "12345"}, 
        {"12345. ", ParseResult::error, ""}, 
        {"Null", ParseResult::error, ""}, 
        {"nUll", ParseResult::error, ""}, 
        {"n", ParseResult::partial, ""}, 
        {"nullxxx", ParseResult::ok, "null"}, 
        {R"("abc")", ParseResult::ok, "\"abc\""}, 
        {R"("abc)", ParseResult::partial, ""}, 
        {R"({"k1":"v1"})", ParseResult::ok, R"({"k1":"v1"})"},
        {R"({"k1":"v1", "k2": "v2"})", ParseResult::ok, R"({"k1":"v1","k2":"v2"})"}, 
        {R"({"k1":"v1", "o1": {"k2": "v2"}})", ParseResult::ok, R"({"k1":"v1","o1":{"k2":"v2"}})"}, 
        {R"({"k1":"v1", "a1": ["v2", "v3"]})", ParseResult::ok, R"({"a1":["v2","v3"],"k1":"v1"})"} 

    };
    bool sync_test{true};
    if (sync_test) { 
        JSON_LOG_DEBUG("FIRST test");
        for(const auto& t: tests) {
            JSON_LOG_DEBUG("testing input: {} **********************", t.input);
            libacpp::json::JsonConsumer consumer;
            ValueParser<JsonConsumer> sp(consumer);
            const char* p = t.input.data();
            auto r = sp.parse(p, p + t.input.size());
            EXPECT_EQ(r, t.parseResult) << "test 1 " << t.input << " " << (int)r << " " <<  (int)t.parseResult;
            std::stringstream ss;
            to_string(consumer.root(), ss);

            std::string result = ss.str();;
            JSON_LOG_DEBUG("result: {}", result);
            if (r == ParseResult::ok) {
                EXPECT_EQ(result, t.result) << "test 1 " << t.input << " " <<  t.result;            
            }
        }
    }
    // if (async_test) { 
    //     JSON_LOG_DEBUG("SECOND test");
    //     int c =0;
    //     for(const auto& t: tests) {
    //         JSON_LOG_DEBUG("testing input: {}", t.input);
    //         ++c;
    //         JsonConsumer consumer;
    //         ValueParser<JsonConsumer> sp(consumer);
    //         const char* p = t.input.data();
    //         const char* end = p + t.input.size();
    //         ParseResult r = ParseResult::partial;
    //         for(char c: t.input) {
    //             char b[2];
    //             b[0] = c;
    //             b[1] = 0;
    //             const char* p = &b[0];
    //             const char* end = p+1;
    //             r = sp.parse(p, end);
    //             if (r != ParseResult::partial)
    //                 break;
    //         }

    //         EXPECT_EQ(r, t.parseResult) << "test 1 " << t.input << " " << (int)r << " " <<  (int)t.parseResult << " test: " << c; 
    //         std::string result = to_string(consumer);
    //         JSON_LOG_DEBUG("result: {}", result);
    //         if (r == ParseResult::ok) {
    //             EXPECT_EQ(result, t.result) << "test 1 " << t.input << " " <<  t.result;            
    //         }
    //     }
    // }

}
