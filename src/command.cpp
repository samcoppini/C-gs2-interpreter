#include "command.hpp"

namespace gs2 {

Command::Command(std::vector<uint8_t> bytes):
    _bytes(std::move(bytes))
{}

const std::vector<uint8_t> &Command::getBytes() const {
    return _bytes;
}

bool isStringEnd(const uint8_t byte) {
    return byte == 0x05 || byte == 0x06 || (byte >= 0x9b && byte <= 0x9f);
}

} // namespace gs2
