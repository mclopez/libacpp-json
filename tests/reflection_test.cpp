
#include <gtest/gtest.h> 
#include "libacpp-json/log.h"
#include <libacpp-json/reflection.h>
#include <functional>
#include <type_traits>

class test1{
public:
    int m1 = 1;
    int m2() {return m2_;}
    void m2(int v) {m2_ = v;}
    int m3() {return m3_;}
    void set_m3(int v) {m3_ = v;}
    void set_m4(int v) {m4_ = v;}
    int m4_ = 4;
    auto s() {return s_;}
    void set_s(std::string s) {s_ = s;}
private:
    int m2_ = 2;
    int m3_ = 3;
    std::string s_ = "hola";
};

template<> 
class libacpp::json::reflection::editor<test1> {
public:
    using properties = std::tuple<
        property<"m1", &test1::m1, &test1::m1>,
        property<"m3", &test1::m3, &test1::set_m3>,
        property<"m4", &test1::m4_, &test1::set_m4>,
        property<"s", &test1::s, &test1::set_s>
    >;
};


TEST(RelectionTests, test1) {
    using namespace libacpp::json::reflection;

    test1 t1;
    
    using prop_m3 = property<"m3", &test1::m3, &test1::set_m3>;
    EXPECT_EQ(prop_m3::get(t1), 3);
    auto v3 = prop_m3::get(t1);
    prop_m3::set(t1, v3*2);
    EXPECT_EQ(prop_m3::get(t1), 6);


    using prop_m4 = property<"m4", &test1::m4_, &test1::set_m4>;
    auto v4 = prop_m4::get(t1);
    EXPECT_EQ(prop_m4::get(t1), 4);
    prop_m4::set(t1, v4*2);
    EXPECT_EQ(prop_m4::get(t1), 8);


    using prop_s = property<"s", &test1::s, &test1::set_s>;
    auto v5 = prop_s::get(t1);
    EXPECT_EQ(prop_s::get(t1), "hola");
    prop_s::set(t1, "mundo");
    EXPECT_EQ(prop_s::get(t1), "mundo");


    EXPECT_EQ((get_prop_value<test1, int>(t1, "m3")), 6);
    set_prop_value(t1, "m3", 7);
    EXPECT_EQ((get_prop_value<test1, int>(t1, "m3")), 7);
    
}
