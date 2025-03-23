//  Copyright Marcos Cambón-López 2024.

// Distributed under the Mozilla Public License Version 2.0.
//    (See accompanying file LICENSE or copy at
//          https://www.mozilla.org/en-US/MPL/2.0/)

#include <libacpp-json/parser.h>

namespace libacpp::json {



ParseResult LiteralParser::parse(const char*& p, const char* end) {
    while(p != end) {
        std::cout << "LiteralParser::parse " << literal_ << " " << *p << " p:"  << (void*)p << " end:"  << (void*)end << std::endl;
        if (*p != literal_[pos_]) {
            //std::cout << "LiteralParser::parse " << literal_ << " error" << std::endl;
            return ParseResult::error;
        }
        if(pos_ == literal_.size()-1){
            ++p;
            //std::cout << "LiteralParser::parse "<< literal_ << " ok" << std::endl;
            return ParseResult::ok;
        }
        ++pos_;
        ++p;
    }
    return ParseResult::partial;
}

} // namespace libacpp::json
