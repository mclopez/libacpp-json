//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#pragma once

#include <string>
#include <vector>

namespace libacpp::json {

enum class ParseResult {ok, partial, error};


class JsonParser {
public:    
    virtual ParseResult parse(const char*& p, const char* end) = 0;
};

class JsonVisitorBase {
public:

    virtual std::string& current_key() =0;
//    virtual void current_key(std::string&) =0;

    virtual void string_value(std::string& key, std::string& value) =0;
    virtual void begin_object() =0;
    virtual void end_object() =0;

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

private:
    enum class Status {begin, middle, escape, unicode, uni4end};
    Status status_;
    std::string& result_;
    int uniCount_;
    uint32_t unicode_;
};

class IntegerParser: public JsonParser {
public:
    IntegerParser(int& result, int& frac, int& exp):result_(result), frac_(frac), exp_(exp), status_(Status::begin), sign_(1){}
    ParseResult parse(const char*& p, const char* end) override; 

private:
    enum class Status {begin, integer, frac, frac2, exp, exp_sign, exp_val1, exp2};
    Status status_;
    int& result_;
    int sign_;
    int& frac_;
    int& exp_;
};


}// namespace libacpp::json