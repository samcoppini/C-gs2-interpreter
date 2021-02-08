#pragma once

#include "block.hpp"

#include <cstdint>
#include <vector>
#include <variant>

namespace gs2 {

class GS2Context;

class Command {
    private:
        std::variant<
            std::vector<uint8_t>,
            Block
        > _command;

        static void executeBytes(const std::vector<uint8_t> &bytes, GS2Context &gs2);

    public:
        Command(std::vector<uint8_t> bytes);
        Command(Block block);

        bool operator!=(const Command &rhs) const;

        void execute(GS2Context &gs2) const;

        bool isBytes() const;
        const std::vector<uint8_t> &getBytes() const;

        bool isBlock() const;
        const Block &getBlock() const;
};

constexpr uint8_t STRING_START_CMD = 0x04;
constexpr uint8_t BLOCK_START_CMD = 0x08;
constexpr uint8_t BLOCK_END_CMD = 0x09;
constexpr uint8_t MAP_BLOCK_CMD = 0xfe;
constexpr uint8_t FILTER_BLOCK_CMD = 0xff;

constexpr uint8_t PUSH_BYTE_CMD = 0x01;
constexpr uint8_t PUSH_SHORT_CMD = 0x02;
constexpr uint8_t PUSH_INT_CMD = 0x03;
constexpr uint8_t PUSH_CHAR_CMD = 0x07;
constexpr uint8_t SPLIT_STRING_BYTE = 0x07;

bool isStringEnd(uint8_t byte);

} // namespace gs2
