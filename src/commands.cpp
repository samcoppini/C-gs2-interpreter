#include "commands.hpp"
#include "gs2context.hpp"
#include "gs2exception.hpp"
#include "utils.hpp"

#include <cassert>
#include <regex>

namespace gs2 {

// 0x30 - add / catenate
void catenate(GS2Context &gs2) {
    auto y = gs2.pop();
    auto x = gs2.pop();

    if (x.isNumber() && y.isNumber()) {
        gs2.push(x.getNumber() + y.getNumber());
    }
    else if (x.isList() && y.isList()) {
        x.getList().concat(y.getList());
        gs2.push(std::move(x));
    }
    else if (x.isBlock() && y.isBlock()) {
        x.getBlock().concat(y.getBlock());
        gs2.push(std::move(x));
    }
    else if (x.isList()) {
        x.getList().add(std::move(y));
        gs2.push(std::move(x));
    }
    else if (y.isList()) {
        y.getList().insert(y.getList().begin(), std::move(x));
        gs2.push(std::move(y));
    }
    else {
        throw GS2Exception{"Unsupported types for add / catenate!"};
    }
}

// 0x0c - empty-block
void emptyBlock(GS2Context &gs2) {
    gs2.push(Block{});
}

// 0x0b - empty-list
void emptyList(GS2Context &gs2) {
    gs2.push(List{});
}

// 0x2a - double / lines
void lines(GS2Context &gs2) {
    auto value = gs2.pop();

    if (value.isNumber()) {
        gs2.push(value.getNumber() * 2);
    }
    else if (value.isList()) {
        auto &list = value.getList();
        
        if (!list.empty()) {
            auto &back = list.back();
            if (back.isNumber() && back.getNumber() == '\n') {
                list.pop();
            }
        }

        List newlineList;
        newlineList.add('\n');
        gs2.push(split(list, newlineList));
    }
    else {
        throw GS2Exception{"Unsupported type for double / lines!"};
    }
}

// 0x20 - negate / reverse / evaluate
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

// 0x0a - new-line
void newLine(GS2Context &gs2) {
    List list;
    list.add('\n');
    gs2.push(std::move(list));
}

// 0x50 - pop
void pop(GS2Context &gs2) {
    gs2.pop();
}

// 0x51 - pop2
void pop2(GS2Context &gs2) {
    gs2.pop();
    gs2.pop();
}

// 0x56 - read-num
void readNum(GS2Context &gs2) {
    auto str = makeString(gs2.pop());
    auto numberRegex = std::regex{"-?[0-9]+"};
    std::smatch match;

    if (!std::regex_search(str, match, numberRegex)) {
        throw GS2Exception{"Unable to read a number!"};
    }

    gs2.push(std::stoll(match.str()));
}

// 0x57 - read-nums
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

// 0x52 - show
void show(GS2Context &gs2) {
    auto val = gs2.pop();
    gs2.push(makeList(val.str()));
}

// 0x0d - space
void space(GS2Context &gs2) {
    List list;
    list.add(' ');
    gs2.push(std::move(list));
}

} // namespace gs2
