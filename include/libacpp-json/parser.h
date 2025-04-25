//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#pragma once
//TODO: REMOVE this
#include <iostream>

#include "log.h"

namespace libacpp::json {

enum class ParseResult {ok, partial, error};

std::string to_string(ParseResult r);


#if __cpp_concepts
    #define REQUIRES(...) requires __VA_ARGS__
#else
    #define REQUIRES(...) // No-op if concepts are not supported
#endif

#if __cpp_concepts 

template <typename T, bool is_key>
concept StringConsumerConcept = 
    (requires(T t) {
        
        t.key_begin();
        t.add_char_key('a');
        t.key_end();    

        // t.string_begin(true);
        // t.add_char_string('a');
        // t.string_end();    
    } && (is_key))
    || 
    (requires(T t) {
        t.string_begin();
        t.add_char_string('a');
        t.string_end();    
    } && (!is_key));


template <typename T>
concept NumberConsumerConcept = requires(T t) {
    t.number_begin();
    t.sing(-1);
    t.add_char_int('0');
    t.add_char_frac('0');
    t.add_char_exp('0');
    t.number_end();
};

template <typename T>
concept ValueConsumerConcept = requires(T t) {
    t.set_bool(true);
    t.set_null();
    (typename T::NumberConsumerType&)t.number_consumer();
    (typename T::StringConsumerType&)t.string_consumer();
    (typename T::ObjectConsumerType&)t.object_consumer();
    (typename T::ArrayConsumerType&)t.array_consumer();    
};  


template <typename T>
concept KeyValueConsumerConcept = requires(T t) {
    (typename T::KeyConsumerType&)t.key_consumer();
    (typename T::ValueConsumerType&)t.value_consumer();
};  



template <typename T>
concept ObjectConsumerConcept = requires(T t) {
    t.object_begin();
    t.object_end();
    (typename T::KeyConsumerType&)t.key_consumer();
    (typename T::ValueConsumerType&)t.value_consumer();
};  

template <typename T>
concept ArrayConsumerConcept = requires(T t) {
    t.array_begin();
    t.array_end();
    (typename T::ValueConsumerType&)t.value_consumer();
};  

#endif



template<typename Consumer, bool IsKey = false>
REQUIRES( StringConsumerConcept<Consumer, IsKey> )
class StringParser 
{
public:
    StringParser(Consumer& consumer, bool is_key = false):status_(Status::begin), uniCount_(0), unicode_(1), consumer_(consumer) {}
    ParseResult parse(const char*& p, const char* end);
    void reset() { status_ = Status::begin;}
    Consumer& consumer() {return consumer_;}
private:
    enum class Status {begin, middle, escape, unicode, uni4end};
    void add_char(char c);
    Status status_;
    int uniCount_;
    uint32_t unicode_;
    Consumer& consumer_;
};



template <typename Consumer>
REQUIRES( ValueConsumerConcept<Consumer> )
class NumberParser  {
public:
    NumberParser(Consumer& consumer): status_(Status::begin), consumer_(consumer){}
    ParseResult parse(const char*& p, const char* end); 
    void reset() { status_ = Status::begin;}
    Consumer& consumer() { return consumer_; }
private:
    enum class Status {begin, integer, frac, frac2, exp, exp_first_digit, exp_digits};
    Status status_;
    Consumer& consumer_;
};


class LiteralParser {
public:
    LiteralParser(const std::string& literal):pos_(0), literal_(literal){}
    ParseResult parse(const char*& p, const char* end);

    void reset() { pos_=0;}

private:
    size_t pos_;
    const std::string& literal_;
};


enum class ValueType {undef, nil, boolean, number, string, object, array};

template<typename Consumer> 
REQUIRES(ObjectConsumerConcept<Consumer>)
class ObjectParser;

template<typename Consumer> 
REQUIRES(ArrayConsumerConcept<Consumer>)
class ArrayParser;

template<typename Consumer>
REQUIRES(ValueConsumerConcept<Consumer>)
class ValueParser {
public:
    ValueParser(Consumer& consumer): 
        status_(Status::begin),
        null_parser_(null_val),true_parser_(true_val), false_parser_(false_val),
        consumer_(consumer),
        number_parser_(consumer.number_consumer()),
        string_parser_(consumer.string_consumer())
        {}
    ParseResult parse(const char*& p, const char* end); 

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
    NumberParser<typename Consumer::NumberConsumerType> number_parser_;
    StringParser<typename Consumer::StringConsumerType, false> string_parser_;

    std::unique_ptr<ObjectParser<typename Consumer::ObjectConsumerType>> object_parser_;
    std::unique_ptr<ArrayParser<typename Consumer::ArrayConsumerType>> array_parser_;

    Consumer& consumer_;
};

inline void log_p(const char*& p, int c) {
    //std::cout <<"log_p " << c << " " << (long int) p << std::endl;
}

class WhiteSpaceParser {
public:
    WhiteSpaceParser() = default;

    ParseResult parse(const char*& p, const char* end)  {
        //std::cout << "WhiteSpaceParser::parse P" << (long int) p  << std::endl;
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


template<typename Consumer>
REQUIRES(KeyValueConsumerConcept<Consumer>)
class KeyValueParser {
public:
    KeyValueParser(Consumer& consumer)
        : status_(Status::ws1), consumer_(consumer), 
        kp_(consumer.key_consumer(), true), 
        vp_(consumer.value_consumer()) {
    }

    ParseResult parse(const char*& p, const char* end);

    void reset() {
        status_ = Status::ws1;
    }

    Consumer& consumer() {return consumer_;}

private:
    enum class Status {ws1, key, ws2, sep, ws3, value};
    Status status_;
    WhiteSpaceParser wsp_;

    StringParser<typename Consumer::KeyConsumerType, true> kp_;
    ValueParser<typename Consumer::ValueConsumerType> vp_;

    Consumer& consumer_;
};


template<typename Consumer>
REQUIRES(ObjectConsumerConcept<Consumer>)
class ObjectParser {
public:
    ObjectParser(Consumer& consumer);
    ParseResult parse(const char*& p, const char* end);
    void reset();
private:
    enum class Status {begin, ws0, key_value, ws1, sep, ws2};
    Status status_;
    KeyValueParser<typename Consumer::KeyValueConsumerType> kvp_;
    WhiteSpaceParser wsp_;
    Consumer& consumer_;
};

template<typename Consumer>
REQUIRES(ArrayConsumerConcept<Consumer>)
class ArrayParser {
public:
    ArrayParser(Consumer& consumer);
    ParseResult parse(const char*& p, const char* end);
    void reset();
private:
    enum class Status {begin, value, ws1, sep, ws2};
    Status status_;
    ValueParser<typename Consumer::ValueConsumerType> vp_;
    WhiteSpaceParser wsp_;
    Consumer& consumer_;
};



} // namespace libacpp::json


#include "parser.inl"
