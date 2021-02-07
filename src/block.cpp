#include "block.hpp"
#include "command.hpp"
#include "gs2exception.hpp"

#include <optional>

namespace gs2 {

namespace {

std::optional<size_t> findUnstartedString(const std::vector<uint8_t> &code) {
    for (size_t i = 0; i < code.size(); ++i) {
        if (isStringEnd(code[i])) {
            return i;
        }

        if (code[i] == STRING_START_CMD) {
            return std::nullopt;
        }
    }

    return std::nullopt;
}

size_t stringLength(const std::vector<uint8_t> &code, size_t start) {
    for (size_t i = start + 1; i < code.size(); ++i) {
        if (isStringEnd(code[i])) {
            return i - start + 1;
        }
    }

    throw GS2Exception{"Unterminated string encountered!"};
}

} // anonymous namespace

Block::Block() {}

Block::Block(const Block &block): _commands(block._commands) {}

Block::Block(Block &&block): _commands(std::move(block._commands)) {}

Block& Block::operator=(const Block &block) {
    _commands = block._commands;
    return *this;
}

Block& Block::operator=(Block &&block) {
    _commands = std::move(block._commands);
    return *this;
}

Block::~Block() {}

Block Block::parseBytes(const std::vector<uint8_t> &code) {
    std::vector<Block> blocks;
    std::vector<Command> final;

    blocks.emplace_back();

    size_t startIndex = 0;

    if (auto stringEnd = findUnstartedString(code); stringEnd) {
        std::vector<uint8_t> string = { STRING_START_CMD };
        string.insert(string.end(), code.begin(), code.begin() + *stringEnd + 1);
        blocks.back().add(Command{std::move(string)});
        startIndex = *stringEnd + 1;
    }

    auto openBlock = [&] (uint8_t cmdByte) {
        blocks.emplace_back();
        final.emplace_back(std::vector<uint8_t>{cmdByte});
    };

    auto closeBlock = [&] {
        if (blocks.size() < 2) {
            throw GS2Exception{"Cannot close an unopened block!"};
        }
        blocks[blocks.size() - 2].add(std::move(blocks.back()));
        blocks.pop_back();
        blocks.back().add(std::move(final.back()));
        final.pop_back();
    };

    for (size_t i = startIndex; i < code.size(); ++i)
    {
        auto addCommand = [&] (const char *name, size_t cmdLen) {
            if (i + cmdLen > code.size()) {
                throw GS2Exception{"File ended before end of " +
                                   std::string{name} + " command"};
            }

            std::vector<uint8_t> cmd{code.begin() + i, code.begin() + cmdLen + i};
            blocks.back().add(Command{std::move(cmd)});
            i += cmdLen - 1;
        };

        switch (code[i]) {
            case PUSH_BYTE_CMD:
                addCommand("byte push", 2);
                break;

            case PUSH_CHAR_CMD:
                addCommand("string push", 2);
                break;

            case PUSH_SHORT_CMD:
                addCommand("short push", 3);
                break;

            case PUSH_INT_CMD:
                addCommand("long push", 5);
                break;

            case STRING_START_CMD:
                addCommand("array push", stringLength(code, i));
                break;

            case BLOCK_START_CMD:
                openBlock(0x00);
                break;

            case MAP_BLOCK_CMD:
                openBlock(0x34);
                break;

            case FILTER_BLOCK_CMD:
                openBlock(0x35);
                break;

            case BLOCK_END_CMD:
                closeBlock();
                break;

            default:
                addCommand("", 1);
                break;
        }
    }

    while (!final.empty()) {
        closeBlock();
    }

    auto block = std::move(blocks[0]);
    return block;
}

void Block::execute(GS2Context &gs2) const {
    for (const auto &command: _commands) {
        command.execute(gs2);
    }
}

void Block::add(Command command) {
    _commands.emplace_back(std::move(command));
}

const std::vector<Command> &Block::getCommands() const {
    return _commands;
}

} // namespace gs2
