#include "block.hpp"
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

Block Block::parseBytes(const std::vector<uint8_t> &code) {
    Block block;

    size_t startIndex = 0;

    if (auto stringEnd = findUnstartedString(code); stringEnd) {
        std::vector<uint8_t> string = { STRING_START_CMD };
        string.insert(string.end(), code.begin(), code.begin() + *stringEnd + 1);
        block.add(Command{std::move(string)});
        startIndex = *stringEnd + 1;
    }

    for (size_t i = startIndex; i < code.size(); ++i)
    {
        auto addCommand = [&] (const std::string &name, size_t cmdLen) {
            if (i + cmdLen > code.size()) {
                throw GS2Exception{"File ended before end of " + name + " command"};
            }

            std::vector<uint8_t> cmd{code.begin() + i, code.begin() + cmdLen + i};
            block.add(Command{std::move(cmd)});
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

            default:
                addCommand("", 1);
                break;
        }
    }

    return block;
}

void Block::add(Command command) {
    _commands.emplace_back(std::move(command));
}

const std::vector<Command> &Block::getCommands() const {
    return _commands;
}

} // namespace gs2
