#include "command.hpp"
#include "commands.hpp"
#include "gs2context.hpp"
#include "gs2exception.hpp"

#include <type_traits>

namespace gs2 {

namespace {

List splitString(std::vector<uint8_t>::const_iterator begin,
                 std::vector<uint8_t>::const_iterator end)
{
    List strings;
    List curString;

    for (auto it = begin; it != end; ++it) {
        if (*it == SPLIT_STRING_BYTE) {
            strings.add(std::move(curString));
        }
        else {
            curString.add(*it);
        }
    }

    strings.add(std::move(curString));

    return strings;
}

} // anonymous namespace

Command::Command(std::vector<uint8_t> bytes):
    _command(std::move(bytes))
{}

Command::Command(Block block):
    _command(std::move(block))
{}

bool Command::operator!=(const Command &rhs) const {
    if (rhs._command.index() != _command.index()) {
        return true;
    }

    return std::visit([&rhs] (const auto &arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, Block>) {
            return arg != rhs.getBlock();
        }
        else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            return arg != rhs.getBytes();
        }
    }, _command);
}

void Command::execute(GS2Context &gs2) const {
    std::visit([&gs2] (const auto &arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            executeBytes(arg, gs2);
        }
        else if constexpr (std::is_same_v<T, Block>) {
            gs2.push(arg);
        }
    }, _command);
}

void Command::executeBytes(const std::vector<uint8_t> &bytes, GS2Context &gs2) {
    switch (bytes[0]) {
        case 0x00:
            // nop
            break;

        case 0x01:
            gs2.push(bytes[1]);
            break;

        case 0x02: {
            int16_t num = (bytes[1] | (bytes[2] << 8));
            gs2.push(num);
            break;
        }

        case 0x03: {
            int32_t num = 0;
            num |= bytes[1] << 0;
            num |= bytes[2] << 8;
            num |= bytes[3] << 16;
            num |= bytes[4] << 24;
            gs2.push(num);
            break;
        }

        case 0x04: {
            auto strings = splitString(bytes.begin() + 1, bytes.end() - 1);

            switch (bytes.back()) {
                case 0x05:
                    for (auto &string: strings) {
                        gs2.push(std::move(string));
                    }
                    break;

                case 0x06:
                    gs2.push(std::move(strings));
                    break;

                default:
                    throw GS2Exception{"Unhandled string end byte: " +
                                       std::to_string(bytes.back())};
            }

            break;
        }

        case 0x07: {
            List list;
            list.add(bytes[1]);
            gs2.push(std::move(list));
            break;
        }

        case 0x0a: newLine(gs2);    break;
        case 0x0b: emptyList(gs2);  break;
        case 0x0c: emptyBlock(gs2); break;
        case 0x0d: space(gs2);      break;
        case 0x10: gs2.push(0);     break;
        case 0x11: gs2.push(1);     break;
        case 0x12: gs2.push(2);     break;
        case 0x13: gs2.push(3);     break;
        case 0x14: gs2.push(4);     break;
        case 0x15: gs2.push(5);     break;
        case 0x16: gs2.push(6);     break;
        case 0x17: gs2.push(7);     break;
        case 0x18: gs2.push(8);     break;
        case 0x19: gs2.push(9);     break;
        case 0x1a: gs2.push(10);    break;
        case 0x1b: gs2.push(100);   break;
        case 0x1c: gs2.push(1000);  break;
        case 0x1d: gs2.push(16);    break;
        case 0x1e: gs2.push(64);    break;
        case 0x1f: gs2.push(256);   break;
        case 0x20: negate(gs2);     break;
        case 0x21: head(gs2);       break;
        case 0x22: tail(gs2);       break;
        case 0x23: abs(gs2);        break;
        case 0x2a: lines(gs2);      break;
        case 0x30: catenate(gs2);   break;
        case 0x34: mod(gs2);        break;
        case 0x50: pop(gs2);        break;
        case 0x51: pop2(gs2);       break;
        case 0x52: show(gs2);       break;
        case 0x54: showLines(gs2);  break;
        case 0x55: showWords(gs2);  break;
        case 0x56: readNum(gs2);    break;
        case 0x57: readNums(gs2);   break;
        case 0x58: showLine(gs2);   break;
        case 0x59: showSpace(gs2);  break;
        case 0x64: sum(gs2);        break;
        case 0x65: product(gs2);    break;
        case 0xb2: counter(gs2);    break;

        default:
            throw GS2Exception{"Unhandled command byte: " + std::to_string(bytes[0])};
    }
}

bool Command::isBytes() const {
    return std::holds_alternative<std::vector<uint8_t>>(_command);
}

const std::vector<uint8_t> &Command::getBytes() const {
    return std::get<std::vector<uint8_t>>(_command);
}

bool Command::isBlock() const {
    return std::holds_alternative<Block>(_command);
}

const Block& Command::getBlock() const {
    return std::get<Block>(_command);
}

bool isStringEnd(const uint8_t byte) {
    return byte == 0x05 || byte == 0x06 || (byte >= 0x9b && byte <= 0x9f);
}

} // namespace gs2
