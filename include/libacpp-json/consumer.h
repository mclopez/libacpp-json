//  Copyright Marcos Cambón-López 2024-2025.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <variant>
#include <memory> 

#include <libacpp-json/parser.h>


namespace libacpp::json {

class JsonValue;
using JsonObject = std::map<std::string, JsonValue>;
using JsonArray  = std::vector<JsonValue>;

class JsonValue: public std::variant<std::string, bool, int, double, std::nullptr_t, JsonObject, JsonArray> {
public:
using variant_type = std::variant<std::string, bool, int, double, std::nullptr_t, JsonObject, JsonArray>;

JsonValue() = default;

template< typename T>
JsonValue(const T& s):variant_type(s){}   


template< typename T>
void operator=(const T& s) { variant_type::operator=(s); }   


 
};

void to_string(JsonValue& jv, std::stringstream& ss) {
    std::visit([&](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, bool>) {
            ss << (value ? "true" : "false");
        } else if constexpr (std::is_same_v<T, int>) {
            ss << std::to_string(value);
        } else if constexpr (std::is_same_v<T, double>) {
            ss << std::to_string(value);
        } else if constexpr (std::is_same_v<T, nullptr_t>) {
            ss <<  "null";
        } else if constexpr (std::is_same_v<T, std::string>) {
            ss <<  '"' << value << '"'; // Already a string
        } else if constexpr (std::is_same_v<T, JsonObject>) {
            ss << "{";
            bool first = true;
            for(auto& p: value)  {
                if (first) { 
                    first = false;
                }
                else {
                    ss << ",";
                }
                ss << '"' << p.first << "\":";
                to_string(p.second, ss);
            }
            ss << "}";
        } else if constexpr (std::is_same_v<T, JsonArray>) {
            ss << "[";
            bool first = true;
            for(auto& p: value)  {
                if (first) { 
                    first = false;
                }
                else {
                    ss << ",";
                }
                to_string(p, ss);
            }
            ss << "]";
        }
    }, jv);
}



class JsonConsumer {
public:
    JsonConsumer(){
        path_.push_back(&root_);
    }

    void add_parent(JsonValue& value) { //TODO: std::move??
        if (auto pv = std::get_if<JsonObject>(&value)) {
            path_.push_back(&value);
        } else if (auto pv = std::get_if<JsonArray>(&value)) {
            path_.push_back(&value);
        }
    }


    template <typename T>
    void add_value(const T& value) { //TODO: std::move??
        if (auto pv = std::get_if<JsonObject>(&parent())) {
            (*pv)[key_] = value;
            add_parent((*pv)[key_]);
        } else if (auto pv = std::get_if<JsonArray>(&parent())) {
            (*pv).push_back(JsonValue{value});
            add_parent((*pv).back());
        } else {
            parent() = value;
        }
    }

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
        add_value(string_value_);

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
        add_value(int_); 
    }

    // End Number Consumer


    int integer() {return int_;}
    int frac() {return frac_;}
    int exp() {return exp_;}



    void set_bool(bool v)  { 
        type_ = ValueType::boolean; 
        bool_value_ = v; 
        add_value(v);
    }

    void set_null() { 
        type_ = ValueType::nil;  
        add_value(nullptr);
    }

    //TODO: remove?
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
        type_ = ValueType::object; 
        add_value(JsonObject{});
    }

    void object_end() {
        object_ = false;
        path_.pop_back();
    }
    //End Object Consumer

    //Begin Array Consumer
    void array_begin() {
        type_ = ValueType::array; 
        add_value(JsonArray{});
    }

    void array_end() {
        path_.pop_back();
    }
    //End Array Consumer
    
    JsonValue& root() { return root_;}
    JsonValue& parent() { return *path_.back();}

private:
    std::string key_;

    int int_ = 0;
    int frac_ = 0;
    int exp_ = 0;
    int sign_ = 1;

    ValueType type_;

    bool bool_value_;
    std::string string_value_;
    bool object_ = false;
    JsonValue root_;
    std::vector<JsonValue*> path_;
    //JsonValue* current_value_ = &root_;
};


}//namespace libacpp::json