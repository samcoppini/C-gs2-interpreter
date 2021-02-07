#include "commands.hpp"
#include "gs2context.hpp"
#include "gs2exception.hpp"

#include <cassert>
#include <regex>

namespace gs2 {

std::string makeString(Value value) {
    std::string str;

    if (value.isNumber()) {
        str += static_cast<char>(value.getNumber());
        return str;
    }
    else if (value.isList()) {
        for (const auto &val: value.getList()) {
            if (!val.isNumber()) {
                throw GS2Exception{"Unable to convert non-number inside list to string!"};
            }
            str += static_cast<char>(val.getNumber());
        }
        return str;
    }
    else {
        throw GS2Exception{"Cannot convert a block to a string!"};
    }
}

// 0x20 command
// negate a number
// reverse a list
// evaluate a block
void negate(GS2Context &gs2) {
    auto value = gs2.pop();

    if (value.isNumber()) {
        gs2.push(-value.getNumber());
    }
    else if (value.isList()) {
        value.getList().reverse();
        gs2.push(std::move(value));
    }
    else {
        assert(value.isBlock());
        value.getBlock().execute(gs2);
    }
}

void readNum(GS2Context &gs2) {
    auto str = makeString(gs2.pop());
    auto numberRegex = std::regex{"-?[0-9]+"};
    std::smatch match;

    if (!std::regex_search(str, match, numberRegex)) {
        throw GS2Exception{"Unable to read a number!"};
    }

    gs2.push(std::stoll(match.str()));
}

// 0x57 command
// read-nums
void readNums(GS2Context &gs2) {
    auto str = makeString(gs2.pop());
    auto numberRegex = std::regex{"-?[0-9]+"};

    auto begin = std::sregex_iterator(str.begin(), str.end(), numberRegex);
    auto end = std::sregex_iterator();

    List numbers;
    for (auto it = begin; it != end; ++it) {
        numbers.add(std::stoll(it->str()));
    }

    gs2.push(std::move(numbers));
}

} // namespace gs2
