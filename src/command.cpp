#include "command.hpp"
#include "gs2context.hpp"
#include "gs2exception.hpp"

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
    _bytes(std::move(bytes))
{}

void Command::execute(GS2Context &gs2) const {
    switch (_bytes[0]) {
        case 0x00:
            // nop
            break;

        case 0x01:
            gs2.push(_bytes[1]);
            break;

        case 0x02: {
            int16_t num = (_bytes[1] | (_bytes[2] << 8));
            gs2.push(num);
            break;
        }

        case 0x03: {
            int32_t num = 0;
            num |= _bytes[1] << 0;
            num |= _bytes[2] << 8;
            num |= _bytes[3] << 16;
            num |= _bytes[4] << 24;
            gs2.push(num);
            break;
        }

        case 0x04: {
            auto strings = splitString(_bytes.begin() + 1, _bytes.end() - 1);

            switch (_bytes.back()) {
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
                                       std::to_string(_bytes.back())};
            }

            break;
        }

        case 0x07: {
            List list;
            list.add(_bytes[1]);
            gs2.push(std::move(list));
            break;
        }

        case 0x10: gs2.push(0);    break;
        case 0x11: gs2.push(1);    break;
        case 0x12: gs2.push(2);    break;
        case 0x13: gs2.push(3);    break;
        case 0x14: gs2.push(4);    break;
        case 0x15: gs2.push(5);    break;
        case 0x16: gs2.push(6);    break;
        case 0x17: gs2.push(7);    break;
        case 0x18: gs2.push(8);    break;
        case 0x19: gs2.push(9);    break;
        case 0x1a: gs2.push(10);   break;
        case 0x1b: gs2.push(100);  break;
        case 0x1c: gs2.push(1000); break;
        case 0x1d: gs2.push(16);   break;
        case 0x1e: gs2.push(64);   break;
        case 0x1f: gs2.push(256);  break;

        default:
            throw GS2Exception{"Unhandled command byte: " + std::to_string(_bytes[0])};
    }
}

const std::vector<uint8_t> &Command::getBytes() const {
    return _bytes;
}

bool isStringEnd(const uint8_t byte) {
    return byte == 0x05 || byte == 0x06 || (byte >= 0x9b && byte <= 0x9f);
}

} // namespace gs2
