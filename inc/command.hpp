#pragma once

#include <cstdint>
#include <vector>

namespace gs2 {

class GS2Context;

class Command {
    private:
        std::vector<uint8_t> _bytes;

    public:
        Command(std::vector<uint8_t> bytes);

        void execute(GS2Context &gs2) const;

        const std::vector<uint8_t> &getBytes() const;
};

constexpr uint8_t STRING_START_CMD = 0x04;
constexpr uint8_t PUSH_BYTE_CMD = 0x01;
constexpr uint8_t PUSH_SHORT_CMD = 0x02;
constexpr uint8_t PUSH_INT_CMD = 0x03;
constexpr uint8_t PUSH_CHAR_CMD = 0x07;
constexpr uint8_t SPLIT_STRING_BYTE = 0x07;

bool isStringEnd(uint8_t byte);

} // namespace gs2
