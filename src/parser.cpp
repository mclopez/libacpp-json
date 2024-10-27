#include <libacpp-json/parser.h>



#include <iostream>
#include <memory>
#include <vector>

//enum class ParseResult {ok, partial, error};

/*
//white space
ParsResult parse_ws(int& status, const char*& p, const char* end) {
    
    while ((*p == ' ' || *p == '\t' ||*p == '\n' ||*p == '\r') && p != end)
        ++p;
    return ParseResult::ok;
}

enum class int StringStatus {begin, midle, end};

ParsResult parse_string(int& status, const char*& p, const char* end) {
    
    while ((*p == ' ' || *p == '\t' ||*p == '\n' ||*p == '\r') && p != end)
        ++p;
    return ParseResult::ok;
}

ParsResult parse_key_value(int& status, const char*& p, const char* end) {
    return ParseResult::ok;
}

*/
/*
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


class StringParser: public JsonParser {
public:
    StringParser(std::string& result):result_(result), status_(Status::begin){

    }
    ParseResult parse(const char*& p, const char* end) override {
        while(p != end) {
            switch(status_) {
                case Status::begin:
                    result_ = "";
                    if (*p != '"') {
                        return ParseResult::error;
                    }
                    status_ = Status::middle;
                    break;
                case Status::middle:
                    if (*p == '"') {
                        ++p;
                        status_  = Status::begin;
                        return ParseResult::ok;
                    }
                    result_ += *p;
                    break;
                default:
                    break;
            }
            ++p;
        }
        return ParseResult::partial;
    }
private:
    enum class Status {begin, middle, end};
    Status status_;
    std::string& result_;
};


class StringValueParser: public JsonParser {
public:
    StringValueParser(JsonVisitorBase& visitor):visitor_(visitor), status_(Status::begin){

    }
    ParseResult parse(const char*& p, const char* end) override {
        while(p != end) {
            switch(status_) {
                case Status::begin:
                    result_ = "";
                    if (*p != '"') {
                        return ParseResult::error;
                    }
                    status_ = Status::middle;
                    break;
                case Status::middle:
                    if (*p == '"') {
                        ++p;
                        visitor_.string_value(visitor_.current_key(), result_);
                        status_ = Status::begin;
                        return ParseResult::ok;
                    }
                    result_ += *p;
                    break;
                default:
                    break;
            }
            ++p;
        }
        return ParseResult::partial;
    }
private:
    enum class Status {begin, middle, end};
    Status status_;
    std::string result_;
    JsonVisitorBase& visitor_;
};


class WhiteSpaceParser: public JsonParser {
public:
    WhiteSpaceParser(){

    }
    ParseResult parse(const char*& p, const char* end) override {
        while ((*p == ' ' || *p == '\t' ||*p == '\n' ||*p == '\r') && p != end)
            ++p;
        return ParseResult::ok;
    }
private:
};

class ValueParser: public JsonParser {
public:
    ValueParser(JsonVisitorBase& visitor):visitor_(visitor) {

    }
    ParseResult parse(const char*& p, const char* end) override;
    void reset(){jp_.release();}
private:
    std::unique_ptr<JsonParser> jp_;
    JsonVisitorBase& visitor_;
};


class KeyValueParser: public JsonParser {
public:
    KeyValueParser(JsonVisitorBase& visitor)
    :   visitor_(visitor), kp_(visitor.current_key()),
        vp_(visitor), status_(Status::ws1) {

    }
    ParseResult parse(const char*& p, const char* end) override {
        ParseResult result = ParseResult::partial;
        while(p != end) {
            std::cout << "(kvp) *p:" << *p << " status_: " << (int)status_ << std::endl;
            switch(status_) {
                case Status::ws1:
                    if (wsp_.parse(p, end) == ParseResult::ok){
                        status_ = Status::key;
                    }
                    break;
                case Status::key:
                    if (kp_.parse(p, end) == ParseResult::ok)
                        status_ = Status::ws2;
                    break;
                case Status::ws2:
                    if (wsp_.parse(p, end) == ParseResult::ok)
                        status_ = Status::sep;
                    break;
                case Status::sep:
                    std::cout << " sep *p:"  << *p << std::endl; 
                    if (*p == ':') {
                        status_ = Status::ws3;
                        ++p;
                    }
                    else 
                        return ParseResult::error;    
                    break;
                case Status::ws3:
                    if (wsp_.parse(p, end) == ParseResult::ok)
                        status_ = Status::value;
                    break;
                case Status::value:
                    result = vp_.parse(p, end);
                    if (result == ParseResult::ok) {
                        status_ = Status::ws1;
                        vp_.reset();
                        return result;
                    }
                    break;
                default:
                    break;
            }
            //++p;
        }
        return result;
    }
private:
    enum class Status {ws1, key, ws2, sep, ws3, value};
    Status status_;
    //std::string key_;
    WhiteSpaceParser wsp_;
    StringParser kp_;
    ValueParser vp_;
    JsonVisitorBase& visitor_;
};

class ObjectParser: public JsonParser {
public:
    ObjectParser(JsonVisitorBase& visitor):visitor_(visitor), kvp_(visitor), status_(Status::begin) {}

    ParseResult parse(const char*& p, const char* end) override {
        ParseResult result = ParseResult::partial;
        while(p != end) {
            std::cout << "(op)*p:" << *p  << " status: " << (int)status_ << std::endl;
            switch(status_) {
                case Status::begin:
                    if (*p != '{') {
                        return ParseResult::error;
                    }
                    ++p;
                    visitor_.begin_object();
                    status_ = Status::key_value;
                    break;
                case Status::key_value:
                    if (kvp_.parse(p, end) == ParseResult::ok) {
                        if (*p == '}') {
                            visitor_.end_object();
                            status_ = Status::begin; 
                            return ParseResult::ok;
                        }
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
                        ++p;
                    }
                    else 
                        return ParseResult::error;    
                    break;
                case Status::ws2:
                    if (wsp_.parse(p, end) == ParseResult::ok) {
                        if(*p == '}') {
                            //std::cout << "*p:"  << *p << std::endl; 
                            return ParseResult::ok;
                        } else {
                            status_ = Status::key_value;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        return result;
    }
private:
    enum class Status {begin, key_value, ws1, sep, ws2};
    Status status_;
    JsonVisitorBase& visitor_;
    KeyValueParser kvp_;
    WhiteSpaceParser wsp_;
};

ParseResult ValueParser::parse(const char*& p, const char* end) {
    if(!jp_) {
        switch (*p)
        {
            case '"':
                jp_ = std::make_unique<StringValueParser>(visitor_);
                break;
            case '{':
                jp_ = std::make_unique<ObjectParser>(visitor_);
                break;
            case '[':
                //jp_ = std::make_unique<StringParser>(value_);
                break;                    
            default:
                return ParseResult::error;
                break;
        }
    }
    ParseResult result = jp_->parse(p, end);
    return result;
}


//g++ main.cpp -o json && ./json
int main() {
    std::cout << "hey!" << std::endl;
    if (false){
        std::string result;
        StringParser sp(result);
        std::string v(R"("hola")");
        const char* p = v.c_str(); 
        sp.parse(p, p + v.size());
        std::cout << "value: " << result << std::endl;
    }
    if (false){
        std::string result;
        StringParser sp(result);
        std::string v(R"("hola")");
        const char* p = v.c_str(); 
        for(int i=0; i<v.size(); ++i) {
            std::cout << "*p: " << *p<< std::endl;
            auto result = sp.parse(p, p +1);
            //++p;
        } 
        std::cout << "value: " << result << std::endl;
    }
    if (false){
        std::string key, value;
        JsonVisitor jsv;
        KeyValueParser sp(jsv);
        std::string v(R"( "key1" : "value1" )");
        const char* p = v.c_str(); 
        auto result = sp.parse(p, p + v.size());
        std::cout << "key: " << key << " value: " << value << " result: " << (int)result <<  std::endl;
    }
    if(false){
        JsonVisitor jsv;
        KeyValueParser sp(jsv);
        std::string v(R"( "key1" : "value1")");
        const char* p = v.c_str();
        for(int i=0; i<v.size(); ++i) {
            std::cout << "*p: " << *p<< std::endl;
            auto result = sp.parse(p, p +1);
            if (result == ParseResult::ok)
                break;
        } 
    }
    if(false){
        JsonVisitor jsv;
        ObjectParser sp(jsv);
        std::string v(R"({ "key1" : "value1" } )");
        const char* p = v.c_str();
//        auto result = sp.parse(p, p + v.size());
        for(int i=0; i<v.size(); ++i) {
            //std::cout << "*p: " << *p<< std::endl;
            auto result = sp.parse(p, p +1);
        } 
        
    }

    if(true){
        JsonVisitor jsv;
        ObjectParser sp(jsv);
        std::string v(R"({"key1":"value1","key2":"value2"})");
        const char* p = v.c_str();
        for(int i=0; i<v.size(); ++i) {
            auto result = sp.parse(p, p +1);
        }         
    }

    if(true){
        JsonVisitor jsv;
        ObjectParser sp(jsv);
        std::string v(R"({"key1":"value1","key2":{"key3":"value3"}})");
        const char* p = v.c_str();
        for(int i=0; i<v.size(); ++i) {
            auto result = sp.parse(p, p +1);
        }         
    }

}
*/

//https://www.rfc-editor.org/rfc/rfc8259


namespace libacpp::json {

/*

		template<typename OutputIterator>
		inline static void write_value_inv(char32_t val, OutputIterator& it) {
			if (val <= 0x007f)	{
				*it = val;
				++it;
			}else if (val <=0x07ff)	{				
				//2 bytes
				*it = (val & 0x3f) | 0x80;
				++it;
				val >>= 6;
				*it = (val & 0x1f) | 0xc0;
				++it;
			}else if (val <=0xffff)	{				
				//3 bytes
				*it = (val & 0x3f) | 0x80;
				++it;
				val >>= 6;
				*it = (val & 0x3f) | 0x80;
				++it;
				val >>= 6;
				*it = (val & 0x0f) | 0xe0;
				++it;
			}else if (val <=0x10ffff)	{
				//4 bytes
				*it = (val & 0x3f) | 0x80;
				++it;
				val >>= 6;
				*it = (val & 0x3f) | 0x80;
				++it;
				val >>= 6;
				*it = (val & 0x3f) | 0x80;
				++it;
				val >>= 6;
				*it = (val & 0x07) | 0xf0;
				++it;
			}else	{
				codification_error e;
				throw e;			
			}
		}
*/

std::vector<uint8_t> unicode_to_tf8(uint32_t codepoint) {
    std::vector<uint8_t> utf8;

    if (codepoint <= 0x7F) {
        // 1-byte sequence: 0xxxxxxx
        utf8.push_back(static_cast<uint8_t>(codepoint));
    } else if (codepoint <= 0x7FF) {
        // 2-byte sequence: 110xxxxx 10xxxxxx
        utf8.push_back(static_cast<uint8_t>((codepoint >> 6) | 0xC0));
        utf8.push_back(static_cast<uint8_t>((codepoint & 0x3F) | 0x80));
    } else if (codepoint <= 0xFFFF) {
        // 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx
        utf8.push_back(static_cast<uint8_t>((codepoint >> 12) | 0xE0));
        utf8.push_back(static_cast<uint8_t>(((codepoint >> 6) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>((codepoint & 0x3F) | 0x80));
    } else if (codepoint <= 0x10FFFF) {
        // 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        utf8.push_back(static_cast<uint8_t>((codepoint >> 18) | 0xF0));
        utf8.push_back(static_cast<uint8_t>(((codepoint >> 12) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>(((codepoint >> 6) & 0x3F) | 0x80));
        utf8.push_back(static_cast<uint8_t>((codepoint & 0x3F) | 0x80));
    } else {
        // Invalid code point
        throw std::invalid_argument("Invalid Unicode code point");
    }

    return utf8;
}

bool is_hex(uint8_t c, uint8_t& r) {
    if ('0' <= c  && c <= '9') {
        r = c - '0';
        return true;
    }
    if ('a' <= c && c <= 'f') {
        r = ((uint8_t)10) + (c - 'a');
        return true;
    }
    if ('A' <= c && c <= 'F') {
        r = ((uint8_t)10) + (c - 'A');
        return true;
    }
    return false;
}

ParseResult StringParser::parse(const char*& p, const char* end) {
    while(p != end) {
        switch(status_) {
            uint8_t v;
            case Status::begin:
                result_ = "";
                if (*p != '"') {
                    status_  = Status::begin;
                    return ParseResult::error;
                }
                status_ = Status::middle;
                break;
            case Status::middle:
                if (*p == '"') {
                    ++p;
                    status_  = Status::begin;
                    return ParseResult::ok;
                } else if (*p == '\\') {
                    status_ = Status::escape;
                } else 
                    result_ += *p;
                break;
            case Status::escape:
                if (*p == 't') {
                    result_ += '\t';
                    status_ = Status::middle;
                } else if (*p == '\\') {
                    result_ += '\\';
                    status_ = Status::middle;
                } else if (*p == 'n') {
                    result_ += '\n';
                    status_ = Status::middle;
                } else if (*p == 'r') {
                    result_ += '\r';
                    status_ = Status::middle;
                } else if (*p == 'u') {
                    status_ = Status::unicode;
                    uniCount_ = 0;
                    unicode_ = 0;
                }
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
                    auto str = unicode_to_tf8(unicode_);
                    for (auto c: str)
                        result_.push_back((char)c);
                    status_ = Status::middle;
                }
                break;    
            default:
                break;
        }
        //std::cout << result_ << std::endl;
        ++p;
    }
    return ParseResult::partial;
}



} // namespace libacpp::json