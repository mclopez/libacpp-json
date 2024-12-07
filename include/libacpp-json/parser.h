//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#pragma once

#include <string>
#include <vector>

namespace libacpp::json {

enum class ParseResult {ok, partial, error};

std::string to_string(ParseResult r);


enum class ValueType {undef, nil, boolean, number, string, object, array};

class Value {
public:
    Value(ValueType type):type_(type){}
    ValueType type() {return type_;}
    virtual std::string to_string()=0;
//    bool operator ==(const Value& v) {return type_ == v.type_};
protected:
    ValueType type_; 
};

// templat<typename V1. typename V2>
// bool operator ==(const Value& v1, const Value& v2) {
//     return type_ == v.type_

// };


class NullValue: public Value {
public:
    NullValue():Value(ValueType::nil){}
    std::string to_string() override {return "null";}
private:
};

//TODO: template impl??
class BoolValue: public Value {
public:
    BoolValue(bool v):Value(ValueType::boolean), value_(v){}
    bool value() const { return value_;}
    std::string to_string() override {return value_?"true":"false";}
private:
    bool value_;
};

class StringValue: public Value {
public:
    StringValue(const std::string& v):Value(ValueType::string), value_(v){}
    std::string value() const { return value_;}
    std::string to_string() override {return std::string("\"") + value_ + "\"";}
private:
    std::string value_;
};

class NumberValue: public Value {
public:
    NumberValue(int i, int f, int e):Value(ValueType::number), integer_(i), fraction_(f), exponent_(e){}
    int value() const { return integer_;}
    std::string to_string() override {return  std::to_string(integer_) + "/" + std::to_string(fraction_) + "/" + std::to_string(exponent_);}
private:
    int integer_;
    int fraction_;
    int exponent_;
};

class JsonParser {
public:    
    virtual ParseResult parse(const char*& p, const char* end) = 0;
};

class JsonVisitorBase {
public:

//    virtual std::string& current_key() =0;
//    virtual void current_key(std::string&) =0;

    virtual void key(std::string&) =0;
    virtual void null_value() =0;
    virtual void string_value(std::string& value) =0;
    virtual void bool_value(bool value) =0;
    virtual void numbrer_value(int integer, int frac, int exp) =0;
    virtual void begin_object() =0;
    virtual void end_object() =0;
    virtual void begin_array() =0;
    virtual void end_array() =0;

private:
};

/*
class JsonVisitor: public JsonVisitorBase {
public:
    std::string& current_key() override { return current_key_;}
//    void current_key(std::string& ck) override { current_key_ = ck;}//TODO: needed???

    void string_value(std::string& key, std::string& value) override {
        std::cout << "JsonVisitor::string_value: ";
        for (int i=0; i<indent_;++i )
            std::cout << '\t';
        std::cout << key << " : " << value << "\n"; 
    }
    void begin_object() override {
        std::cout << "JsonVisitor::begin_object:";
        for (int i=0; i<indent_;++i )
            std::cout << '\t';
        std::cout << current_key_;
        ++indent_;
    }
    void end_object() override {
        std::cout << "JsonVisitor::end_object: ";
        --indent_;
        for (int i=0; i<indent_;++i )
            std::cout << '\t';
        std::cout << current_key_;
    }

private:
    int indent_=0;
    std::string current_key_;
};

*/




std::vector<uint8_t> unicode_to_tf8(uint32_t codepoint);

class StringParser: public JsonParser {
public:
    StringParser(std::string& result):result_(result), status_(Status::begin), uniCount_(0), unicode_(1) {}
    ParseResult parse(const char*& p, const char* end) override; 
    void reset() { status_ = Status::begin;}

private:
    enum class Status {begin, middle, escape, unicode, uni4end};
    Status status_;
    std::string& result_;
    int uniCount_;
    uint32_t unicode_;
};

class NumberParser: public JsonParser {
public:
    NumberParser(int& result, int& frac, int& exp):result_(result), frac_(frac), exp_(exp), status_(Status::begin), sign_(1){}
    ParseResult parse(const char*& p, const char* end) override; 
    void reset() { status_ = Status::begin;}
private:
    enum class Status {begin, integer, frac, frac2, exp, exp_first_digit, exp_digits};
    Status status_;
    int& result_;
    int sign_;
    int& frac_;
    int& exp_;
};

class LiteralParser: public JsonParser {
public:
    LiteralParser(const std::string& literal):pos_(0), literal_(literal){}
    ParseResult parse(const char*& p, const char* end) override; 

    void reset() { pos_=0;}

private:
    size_t pos_;
    const std::string& literal_;
};

class ObjectParser;
class ArrayParser;

class ValueParser: public JsonParser {
public:
    ValueParser(JsonVisitorBase& v)
    :status_(Status::begin),
    null_parser_(null_val),true_parser_(true_val), false_parser_(false_val),
    number_parser_(integer_, frac_, exp_), string_parser_(string_), type_(ValueType::undef), visitor_(v)
    {}
    ParseResult parse(const char*& p, const char* end) override; 
    std::unique_ptr<Value> value() const;

    void reset();
private:
    enum class Status {begin, true_val, false_val, null_val, number, string, object, array};
    Status status_;
    static const std::string null_val;
    static const std::string true_val;
    static const std::string false_val;

    LiteralParser null_parser_;
    LiteralParser true_parser_;
    LiteralParser false_parser_;
    NumberParser number_parser_;
    StringParser string_parser_;
    std::unique_ptr<ObjectParser> object_parser_;
    std::unique_ptr<ArrayParser> array_parser_;

    bool bvalue_;
    int integer_;
    int frac_;
    int exp_;
    std::string string_;

    ValueType type_;
    JsonVisitorBase& visitor_;
};


class WhiteSpaceParser: public JsonParser {
public:
    WhiteSpaceParser() = default;

    ParseResult parse(const char*& p, const char* end)  {
        while (p != end) {
            if (!is_ws(*p)) 
                return ParseResult::ok;
            ++p;
        }
        return ParseResult::partial;
    }

private:
    bool is_ws(char c) {
        return ( c == ' ' || c == '\t' || c == '\n' || c == '\r');
    }
};


class KeyValueParser: public JsonParser {
public:
    KeyValueParser(JsonVisitorBase& v)
    : status_(Status::ws1), kp_(key_), visitor_(v), vp_(v) {

    }
    ParseResult parse(const char*& p, const char* end);
    void reset() {
        key_ = "";
        status_ = Status::ws1;
        vp_.reset();

    }
private:
    enum class Status {ws1, key, ws2, sep, ws3, value};
    Status status_;
    std::string key_;
    WhiteSpaceParser wsp_;
    StringParser kp_;
    ValueParser vp_;
    JsonVisitorBase& visitor_;
};

class ObjectParser: public JsonParser {
public:
    ObjectParser(JsonVisitorBase& visitor);
    ParseResult parse(const char*& p, const char* end) override;
    void reset();
private:
    enum class Status {begin, ws0, key_value, ws1, sep, ws2};
    Status status_;
    JsonVisitorBase& visitor_;
    KeyValueParser kvp_;
    WhiteSpaceParser wsp_;
};

class ArrayParser: public JsonParser {
public:
    ArrayParser(JsonVisitorBase& visitor);
    ParseResult parse(const char*& p, const char* end) override;
    void reset();
private:
    enum class Status {begin, value, ws1, sep, ws2};
    Status status_;
    JsonVisitorBase& visitor_;
    ValueParser vp_;
    WhiteSpaceParser wsp_;
};


}// namespace libacpp::json