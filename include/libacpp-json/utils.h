
#include <cstdint>

#include <vector>

namespace libacpp::json {

bool is_hex(uint8_t c, uint8_t& r);

std::vector<uint8_t> unicode_to_tf8(uint32_t codepoint);

}