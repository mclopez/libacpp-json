//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#include <iostream>

#include <libacpp-json/log.h>
#include <libacpp-json/utils.h>

namespace libacpp::json {



template <typename Consumer, bool is_key>
void StringParser<Consumer, is_key>::add_char(char c) {
    if constexpr (is_key)
        consumer_.add_char_key(c);
    else 
        consumer_.add_char_string(c);    
}

template <typename Consumer, bool is_key>
ParseResult StringParser<Consumer, is_key>::parse(const char*& p, const char* end) {
    while(p != end) {
        std::cout << "StringParser::parse *p: " << *p << " " << (long int)p  << " " << (long int)end << std::endl;
        switch(status_) {
            uint8_t v;
            case Status::begin:
                if constexpr(is_key)
                    consumer_.key_begin();    
                else 
                    consumer_.string_begin();
                if (*p != '"') {
                    status_  = Status::begin;
                    return ParseResult::error;
                }
                status_ = Status::middle;
                break;
            case Status::middle:
                if (*p == '"') {
                    status_  = Status::begin;
                    if constexpr(is_key)
                        consumer_.key_end();
                    else 
                        consumer_.string_end();
                    ++p; log_p(p, 1);
                    return ParseResult::ok;
                } else if (*p == '\\') {
                    status_ = Status::escape;
                } else
                    add_char(*p);
                break;
            case Status::escape:
                if (*p == 't') {
                    add_char('\t');
                    status_ = Status::middle;
                } else if (*p == '\\') {
                    add_char('\\');
                    status_ = Status::middle;
                } else if (*p == 'n') {
                    add_char('\n');
                    status_ = Status::middle;
                } else if (*p == 'r') {
                    add_char('\r');
                    status_ = Status::middle;
                } else if (*p == 'u') {
                    status_ = Status::unicode;
                    uniCount_ = 0;
                    unicode_ = 0;
                }
                //TODO: fix chars beyond Basic Multilingual Plane
                break;    
            case Status::unicode:
                if (is_hex(*p, v) && uniCount_ < 4) {
                    ++ uniCount_;
//                    std::cout << "uniCount_:" << uniCount_ << "unicode_: " << unicode_  << " *p: " << *p << " v:" << (int)v << std::endl;
                    unicode_ = unicode_ << 4 | (v & 0xf);
//                     std::cout << "unicode_(2): " << unicode_ << std::endl;
               } else {
                    status_  = Status::begin;
                    return ParseResult::error;
                } 
                if (uniCount_ == 4) {
                    auto str = unicode_to_utf8(unicode_);
                    for (auto c: str)
                        add_char((char)c);
                    status_ = Status::middle;
                }
                break;    
            default:
                break;
        }
        ++p; log_p(p, 2); 
    }
    return ParseResult::partial;
}


template <typename Consumer>
ParseResult NumberParser<Consumer>::parse(const char*& p, const char* end) {
    while(p != end) {
        std::cout << "NumberParser::parse *p: " << *p << " status_: " << (int)status_  << " "<<(long int) p << " " << (long int) end << std::endl;
        switch(status_) {
            case Status::begin:
                consumer_.number_begin();
                if (*p == '0') {
                    ++p; log_p(p, 3);
                    status_  = Status::begin;
                    return ParseResult::ok;
                } else if ( *p == '-') {
                    consumer_.sign(-1);
                    status_ = Status::integer;
                } else if ( *p == '+') {
                    consumer_.sign(+1);
                    status_ = Status::integer;
                } else if ( '1' <= *p && *p <= '9') {
                    consumer_.add_char_int(*p);
                    status_ = Status::integer;
                } else {
                    return ParseResult::error;
                }

                break;
            case Status::integer:
                if ( '0' <= *p && *p <= '9') {
                    consumer_.add_char_int(*p);
                } else if (*p == '.')  {
                        status_ = Status::frac;    
                }else if (*p == 'e' || *p == 'E') {
                    status_ = Status::exp;
                } else {
                    consumer_.number_end();
                    return ParseResult::ok;
                }

                break;
            case Status::frac:
                if ( '0' <= *p && *p <= '9') {
                    consumer_.add_char_frac(*p);
                    status_ = Status::frac2;
                } else {
                    return ParseResult::error;
                }
                break;
            case Status::frac2:
                if ( '0' <= *p && *p <= '9') {
                    consumer_.add_char_frac(*p);
                }else if (*p == 'e' || *p == 'E') {
                    status_ = Status::exp;
                } else {
                    consumer_.number_end();
                    return ParseResult::ok;
                }
                break;
            case Status::exp:
                //sign_ = +1;
                consumer_.sign(1);
                if ( *p == '-') {
                    consumer_.sign(-1);
                    status_ = Status::exp_first_digit;
                } else if ( *p == '+') {
                    consumer_.sign(1);
                    status_ = Status::exp_first_digit;
                } else if ( '0' <= *p && *p <= '9') {
                    consumer_.add_char_exp(*p);
                    status_ = Status::exp_digits;
                } else {
                    return ParseResult::error;
                }
                break;
            case Status::exp_first_digit:
                if ( '0' <= *p && *p <= '9') {
                    consumer_.add_char_exp(*p);
                    status_ = Status::exp_digits;
                } else {
                    return ParseResult::error;
                }
                break;
            case Status::exp_digits:
                if ( '0' <= *p && *p <= '9') {
                    consumer_.add_char_exp(*p);
                } else {
                    consumer_.number_end();
                    return ParseResult::ok;
                }
                break;
            default:
                break;
        }
        ++p; log_p(p, 4);
    }
    return ParseResult::partial;
}

template<typename Consumer>
const std::string ValueParser<Consumer>::null_val = "null";
template<typename Consumer>
const std::string ValueParser<Consumer>::true_val = "true";
template<typename Consumer>
const std::string ValueParser<Consumer>::false_val = "false";


template <typename Consumer>
void ValueParser<Consumer>::reset() {
    status_ = Status::begin;
    // if (object_parser_)
    //     object_parser_->reset();
    // if (array_parser_)
    //     array_parser_->reset();
    null_parser_.reset();
    true_parser_.reset();
    false_parser_.reset();
    number_parser_.reset();
    string_parser_.reset();

}

struct guard {
    const char* current_ = nullptr;
    void current(const char* p) { 
        if (current_ == p) 
            throw std::runtime_error("infinit loop at " + std::to_string((long int)p));
        else 
            current_ = p;
        }
};

template <typename Consumer>
ParseResult ValueParser<Consumer>::parse(const char*& p, const char* end) {
    //std::cout <<"ValueParser::parse \n";

    ParseResult r = ParseResult::partial;
    guard g;
        std::cout << "ValueParser::parse begin: " << (long int)p << " " << (long int)end << " status_: " << (int)status_ << std::endl;
    while(p < end && r != ParseResult::ok) {
        g.current(p);
        std::cout << "ValueParser::parse " << *p << " p:" << (long int)p  << " end:" << (long int)end  << " r: " << (int)r << " status_: " << (int)status_ << std::endl;
        switch(status_) {
            case Status::begin:
                //null
                status_ = Status::null_val;
                r = null_parser_.parse(p, end);
                if (r == ParseResult::ok){
                    consumer_.set_null();
                } 
                //true
                if (r == ParseResult::error) {
                    status_ = Status::true_val;
                    r = true_parser_.parse(p, end);
                    if (r == ParseResult::ok){
                        consumer_.set_bool(true);
                    } 
                }
                //false
                if (r == ParseResult::error) {
                    status_ = Status::false_val;
                    r = false_parser_.parse(p, end);
                    if (r == ParseResult::ok){
                        consumer_.set_bool(false);
                    } 
                }
                //number
                if (r == ParseResult::error) {
                    status_ = Status::number;
                    r = number_parser_.parse(p, end);
                    if (r == ParseResult::ok) {
                        //consumer_.set_number();
                    }
                }
                //string
                if (r == ParseResult::error) {
                    status_ = Status::string;
                    r = string_parser_.parse(p, end);
                    if (r == ParseResult::ok) {
//                        consumer_.set_string();
                    }
                }
                //object
                if (r == ParseResult::error) {
                    status_ = Status::object;
                    if (!object_parser_) {
                        object_parser_  = std::make_unique<ObjectParser<typename Consumer::ObjectConsumerType>>(consumer_.object_consumer());    
                    }
                    object_parser_->reset();
                    r = object_parser_->parse(p, end);
                }
                //array
                if (r == ParseResult::error) {
                    status_ = Status::array;
                    if (!array_parser_)
                        array_parser_ = std::make_unique<ArrayParser<typename Consumer::ArrayConsumerType>>(consumer_.array_consumer());    
                    array_parser_->reset();    
                    r = array_parser_->parse(p, end);
                }

                //std::cout <<"ValueParser::parse r= " << to_string(r) << " " << (int)status_ <<" " << (int)type_ << std::endl;
                if (r == ParseResult::error){
                    //parse object and value
                    std::cout <<"ERROR ************" << std::endl;
                    return r;
                } 
                break;
            case Status::true_val:
                //std::cout <<"true_parser_.parse" << std::endl;
                r = true_parser_.parse(p, end);
                if (r == ParseResult::ok) {
                    consumer_.set_bool(true);
                }
                break;
            case Status::false_val: 
                r = false_parser_.parse(p, end);
                if (r == ParseResult::ok) {
                    consumer_.set_bool(false);
                }
                break;
            case Status::null_val:
                r = null_parser_.parse(p, end);
                if (r == ParseResult::ok) {
                    consumer_.set_null();
                }
                break;
            case Status::number:
                r = number_parser_.parse(p, end);
                if (r == ParseResult::ok) {
                    //consumer_.set_number();
                }
                break;
            case Status::string:
                std::cout <<"string_parser_.parse p:" << (long int) p  << " e: "<< (long int) end << std::endl;
                r = string_parser_.parse(p, end);
                if (r == ParseResult::ok) {
//                    consumer_.set_string();
                }
                break;
            case Status::object:
                std::cout <<"object_parser_.parse p:" << (long int) p  << " e: "<< (long int) end << std::endl;
                r = object_parser_->parse(p, end);
                if (r == ParseResult::ok) {
                    //type_ = ValueType::object;
                }
                break;
            case Status::array:
                std::cout <<"object_parser_.parse p:" << (long int) p  << " e: "<< (long int) end << std::endl;
                r = array_parser_->parse(p, end);
                if (r == ParseResult::ok) {
                    //type_ = ValueType::object;
                }
                break;
            default:
                break;
        }
        if (r==ParseResult::error)
            return r;
    }
    return r;
}

template<typename Consumer>
ParseResult KeyValueParser<Consumer>::parse(const char*& p, const char* end)  {
    std::cout << "KeyValueParser::parse sep p:"  << (long int)p << " end:"  << (long int)end  << std::endl; 
    while(p != end) {
        std::cout << "KeyValueParser::parse *p: " << *p << " " << (long int)p << " " << (long int)end  << " " << " status_: " << (int)status_ << std::endl;
        ParseResult result = ParseResult::partial;
        switch(status_) {
            case Status::ws1:
                result = wsp_.parse(p, end);
                if ( result == ParseResult::ok){
                    status_ = Status::key;
                }
                break;
            case Status::key:
                result = kp_.parse(p, end);
                if ( result == ParseResult::ok){
                    status_ = Status::ws2;
                }
                break;
            case Status::ws2:
                std::cout << "KeyValueParser::parse ws2 *p:"  << *p << std::endl; 
                result = wsp_.parse(p, end);
                if (result == ParseResult::ok)
                    status_ = Status::sep;
                break;
            case Status::sep:
                if (*p == ':') {
                    // we have separator, key is ok. we could set that in the StringParser
                    status_ = Status::ws3;
                    ++p;  log_p(p, 5);
                    if (p == end)
                        return result;    
                }
                else 
                    return ParseResult::error;    
                break;
            case Status::ws3:
                result = wsp_.parse(p, end);
                if (result == ParseResult::ok) {
                    vp_.reset();
                    status_ = Status::value;
                }
                break;
            case Status::value:
                std::cout << "KeyValueParser::parse value *p:"  << *p << std::endl; 
                result = vp_.parse(p, end);
                if (result == ParseResult::ok) {
                    status_ = Status::ws1;
                    //vp_.reset();
                    return result;
                }
                break;
            default:
                break;
        }
        if (result == ParseResult::error) {
            std::cout << "KeyValueParser::parse error" << std::endl; 
            return result;    
        }
    }
    return ParseResult::partial;
}


template<typename Consumer>
void ObjectParser<Consumer>::reset() {
    status_ = Status::begin;
}

template<typename Consumer>
ObjectParser<Consumer>::ObjectParser(Consumer& consumer)
:status_(Status::begin), consumer_(consumer), kvp_(consumer) {
    std::cout << "ObjectParser::ObjectParser  NEW"  << std::endl;
}


template<typename Consumer>
ParseResult ObjectParser<Consumer>::parse(const char*& p, const char* end) {
    std::cout << "ObjectParser::parse *** p: " << (long int)p << " end: " << (long int)end << std::endl;
    ParseResult result = ParseResult::partial;
    while(p != end) {
        std::cout << "ObjectParser::parse *p: " << *p  << " status: " << (int)status_ << " p " << (long int)p << " end: " << (long int)end << std::endl;
        ParseResult r;
        switch(status_) {
            case Status::begin:
                if (*p != '{') {
                    return ParseResult::error;
                }
                consumer_.object_begin();
                status_ = Status::ws0;
                ++p;  log_p(p, 6);
                if (p == end)
                    return result;
                break;
            case Status::ws0:
                r = wsp_.parse(p, end);
                if ( r == ParseResult::ok) {
                    if(*p == '}') {
                        //std::cout << "*p:"  << *p << std::endl; 
                        return ParseResult::ok;
                    } else {
                        kvp_.reset(); //TODO: needed?
                        status_ = Status::key_value;
                    }
                } else if ( r == ParseResult::error) {
                    return ParseResult::error;
                }
                break;
            case Status::key_value:
                std::cout << "ObjectParser::parse keyvalue (1) *p: " << *p  << " status: " << (int)status_  << " p:" << (long int) p << std::endl;
                r = kvp_.parse(p, end);
                std::cout << "ObjectParser::parse keyvalue (2) *p: " << *p  << " status: " << (int)status_  << " p:" << (long int) p << std::endl;
                if (r == ParseResult::ok) {
                    if (*p == '}') {
                        consumer_.object_end();
                        status_ = Status::begin; 
                        return ParseResult::ok;
                    }
                    // if (*p == ']') {
                    //     visitor_.end_array();
                    //     status_ = Status::begin; 
                    //     return ParseResult::ok;
                    // }
                    status_ = Status::ws1;
                } else if (r == ParseResult::error){
                    JSON_LOG_TRACE("ObjectParser::parse  ERROR **************");
                    return ParseResult::error;
                }
                break;
            case Status::ws1:
                r = wsp_.parse(p, end);
                if ( r == ParseResult::ok) {
                    status_ = Status::sep;
                    if (*p == '}') {
                        consumer_.object_end();
                        status_ = Status::begin; 
                        return ParseResult::ok;
                    }
                }
                else if (r==ParseResult::error)
                    return ParseResult::error;    
                break;
            case Status::sep:
                //std::cout << "*p:"  << *p << std::endl; 
                if (*p == ',') {
                    status_ = Status::ws2;
                    ++p; log_p(p, 7);
                    if (p == end)
                        return result;
                } else if (*p == '}') {
                    ++p; log_p(p, 8);
                    if (p == end)
                        return result;
                    return ParseResult::ok;    
                } else 
                    return ParseResult::error;    
                break;
            case Status::ws2:
                r = wsp_.parse(p, end);
                if ( r == ParseResult::ok) {
                    if(*p == '}') {
                        //std::cout << "*p:"  << *p << std::endl; 
                        return ParseResult::ok;
                    } else {
                        kvp_.reset(); //TODO: needed?
                        status_ = Status::key_value;
                    }
                } else if ( r == ParseResult::error) {
                    return ParseResult::error;
                }
                break;
            default:
                break;
        }
    }
    return result;
}


template<typename Consumer>
ArrayParser<Consumer>::ArrayParser(Consumer& consumer)
:consumer_(consumer), vp_(consumer), status_(Status::begin) {
    std::cout << "ArrayParser::ArrayParser  NEW"  << std::endl;
}

template<typename Consumer>
void ArrayParser<Consumer>::reset() {
    status_ = Status::begin;
}


template<typename Consumer>
ParseResult ArrayParser<Consumer>::parse(const char*& p, const char* end) {
    std::cout << "ArrayParser::parse p: " << (long int) p << " end: " << (long int)end << std::endl;
    ParseResult result = ParseResult::partial;
    while(p != end) {
        std::cout << "ArrayParser::parse *p: " << *p  << " status: " << (int)status_ << " p: " << (long int) p << " end: " << (long int)end << std::endl;
        switch(status_) {
            case Status::begin:
                if (*p != '[') {
                    return ParseResult::error;
                }
                consumer_.array_begin();
                status_ = Status::value;
                ++p; log_p(p, 9);
                if (p == end)
                    return ParseResult::partial;
                break;
            case Status::value:
                        std::cout << "ArrayParser::parse value(1) *********** end p" << (long int) p << std::endl;
                if (vp_.parse(p, end) == ParseResult::ok) {
                    // if (*p == ']') {
                    //     std::cout << "ArrayParser::parse value(2) *********** end p" << (long int) p << std::endl;
                    //     consumer_.array_end();
                    //     status_ = Status::begin; 
                    //     ++p; log_p(p, 10);
                    //     return ParseResult::ok;
                    // }
                    std::cout << "ArrayParser::parse value *********** " << std::endl;
                    status_ = Status::ws1;
                }
                break;
            case Status::ws1:
                if (wsp_.parse(p, end) == ParseResult::ok)
                    status_ = Status::sep;
                break;
            case Status::sep:
                //std::cout << "*p:"  << *p << std::endl; 
                if (*p == ',') {
                    status_ = Status::ws2;
                    ++p; log_p(p, 11);
                    if (p == end)
                        return ParseResult::partial;
                } else if (*p == ']') {
                    ++p; log_p(p, 12);
                    consumer_.array_end();
                    return ParseResult::ok;    
                } else 
                    return ParseResult::error;    
                break;
            case Status::ws2:

                if (wsp_.parse(p, end) == ParseResult::ok) {
                    if(*p == ']') {
                        ++p; log_p(p, 13);
                        consumer_.array_end();
                        //std::cout << "*p:"  << *p << std::endl; 
                        return ParseResult::ok;
                    } else {
                        vp_.reset(); //TODO: needed?
                        status_ = Status::value;
                    }
                }
                break;
            default:
                break;
        }
    }
    return result;
}

}// namespace libacpp::json

