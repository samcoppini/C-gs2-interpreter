#include "commands.hpp"
#include "gs2context.hpp"
#include "gs2exception.hpp"
#include "utils.hpp"

#include <cassert>
#include <regex>

namespace gs2 {

// 0x22 - abs / init
void abs(GS2Context &gs2) {
    auto value = gs2.pop();

    if (value.isNumber()) {
        auto &num = value.getNumber();
        if (num < 0) {
            num *= -1;
        }
        gs2.push(value);
    }
    else if (value.isList()) {
        if (!value.getList().empty()) {
            value.getList().pop();
        }
        gs2.push(std::move(value));
    }
    else {
        throw GS2Exception{"Blocks are not supported for abs / init"};
    }
}

// 0x86 - ascii-digits
void asciiDigits(GS2Context &gs2) {
    List list;
    for (int i = '0'; i <= '9'; i++) {
        list.add(i);
    }
    gs2.push(std::move(list));
}

// 0x30 - add / catenate
void catenate(GS2Context &gs2) {
    auto y = gs2.pop();
    auto x = gs2.pop();

    if (x.isNumber() && y.isNumber()) {
        x.getNumber() += y.getNumber();
        gs2.push(std::move(x));
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

// 0xb2 - counter
void counter(GS2Context &gs2) {
    gs2.push(gs2.getAndIncCounter());
}

// 0x40 - dup
void dup(GS2Context &gs2) {
    gs2.dup(0);
}

// 0x41 - dup2
void dup2(GS2Context &gs2) {
    gs2.dup(1);
    gs2.dup(1);
}

// 0x0c - empty-block
void emptyBlock(GS2Context &gs2) {
    gs2.push(Block{});
}

// 0x0b - empty-list
void emptyList(GS2Context &gs2) {
    gs2.push(List{});
}

// 0x32 - mul / join / times / fold
void fold(GS2Context &gs2) {
    auto y = gs2.pop();
    auto x = gs2.pop();

    if (x.isNumber() && !y.isNumber()) {
        std::swap(x, y);
    }
    else if (x.isBlock() && y.isList()) {
        std::swap(x, y);
    }

    if (x.isNumber() && y.isNumber()) {
        x.getNumber() *= y.getNumber();
        gs2.push(std::move(x));
    }
    else if (x.isList() && y.isList()) {
        gs2.push(join(std::move(x.getList()), y.getList()));
    }
    else if (x.isList() && y.isNumber()) {
        auto &list = x.getList();
        auto &num = y.getNumber();

        List multipliedList;
        while (num-- > 0) {
            for (const auto &val: list) {
                multipliedList.add(val);
            }
        }

        gs2.push(std::move(multipliedList));
    }
    else if (x.isBlock() && y.isNumber()) {
        auto &num = y.getNumber();
        auto &block = x.getBlock();
        while (num-- > 0) {
            block.execute(gs2);
        }
    }
    else if (x.isList() && y.isBlock()) {
        auto &list = x.getList();
        auto &block = y.getBlock();

        if (list.empty()) {
            throw GS2Exception{"Cannot fold an empty list!"};
        }
        gs2.push(std::move(list[0]));
        for (auto it = list.begin() + 1; it != list.end(); ++it) {
            gs2.push(std::move(*it));
            block.execute(gs2);
        }
    }
    else {
        throw GS2Exception{"Unsupported types for multiply / join / times / fold"};
    }
}

// 0x21 - bnot / head
void head(GS2Context &gs2) {
    auto value = gs2.pop();

    if (value.isNumber()) {
        auto &num = value.getNumber();
        num += 1;
        num *= -1;
        gs2.push(std::move(value));
    }
    else if (value.isList()) {
        auto &list = value.getList();
        if (list.empty()) {
            throw GS2Exception{"Cannot get the head of an empty list!"};
        }
        gs2.push(std::move(list[0]));
    }
    else {
        throw GS2Exception{"Unsupported type for bnot / head"};
    }
}

// 0x24 - digits / last
void last(GS2Context &gs2) {
    auto value = gs2.pop();

    if (value.isNumber()) {
        List digits;
        auto numStr = value.getNumber().str();
        for (auto &digit: numStr) {
            if (!std::isdigit(digit)) {
                continue;
            }
            digits.add(digit - '0');
        }
        gs2.push(std::move(digits));
    }
    else if (value.isList()) {
        gs2.push(value.getList().pop());
    }
    else {
        throw GS2Exception{"Blocks are not supported for digits / last"};
    }
}

// 0x2a - double / lines
void lines(GS2Context &gs2) {
    auto value = gs2.pop();

    if (value.isNumber()) {
        value.getNumber() *= 2;
        gs2.push(std::move(value));
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

// 0x85 - lowercase-alphabet
void lowercaseAlphabet(GS2Context &gs2) {
    List list;
    for (int i = 'a'; i <= 'z'; i++) {
        list.add(i);
    }
    gs2.push(std::move(list));
}

// 0x34 - mod / step / clean-split / map
void mod(GS2Context &gs2) {
    auto y = gs2.pop();
    auto x = gs2.pop();

    if (!x.isList() && y.isList()) {
        std::swap(x, y);
    }

    if (x.isNumber() && y.isNumber()) {
        x.getNumber() %= y.getNumber();
        gs2.push(std::move(x));
    }
    else if (x.isList() && y.isNumber()) {
        gs2.push(stepOver(std::move(x.getList()), y.getNumber().convert_to<int64_t>()));
    }
    else if (x.isList() && y.isList()) {
        gs2.push(split(std::move(x.getList()), y.getList(), true));
    }
    else if (x.isList() && y.isBlock()) {
        gs2.do_map(y.getBlock(), std::move(x.getList()));
    }
    else {
        throw GS2Exception{"Unsupported types for mod/step/clean-split/map!"};
    }
}

// 0x20 - negate / reverse / evaluate
void negate(GS2Context &gs2) {
    auto value = gs2.pop();

    if (value.isNumber()) {
        value.getNumber() *= -1;
        gs2.push(std::move(value));
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

// 0x87 - printable-ascii
void printableAscii(GS2Context &gs2) {
    List list;
    for (int i = 32; i <= 126; i++) {
        list.add(i);
    }
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

// 0x65 - product / odd
void product(GS2Context &gs2) {
    auto val = gs2.pop();

    if (val.isNumber()) {
        gs2.push(val.getNumber() % 2 == 0 ? 0 : 1);
    }
    else if (val.isList()) {
        Value::IntType sum = 1;

        for (const auto &numVal: val.getList()) {
            if (!numVal.isNumber()) {
                throw GS2Exception{"Cannot sum a list with non-numbers!"};
            }
            sum *= numVal.getNumber();
        }

        gs2.push(sum);
    }
    else {
        throw GS2Exception{"Cannot perform sum/even on a block!"};
    }
}

// 0x2e - range / length
void range(GS2Context &gs2) {
    auto val = gs2.pop();

    if (val.isNumber()) {
        auto &endVal = val.getNumber();

        List list;
        for (uint64_t i = 0; i < endVal; i++) {
            list.add(i);
        }

        gs2.push(std::move(list));
    }
    else if (val.isList()) {
        gs2.push(val.getList().size());
    }
    else {
        throw GS2Exception{"Block type is not valid for range / length"};
    }
}

// 0x2f - range1
void range1(GS2Context &gs2) {
    auto val = gs2.pop();

    if (val.isNumber()) {
        auto &endVal = val.getNumber();

        List list;
        for (uint64_t i = 1; i <= endVal; i++) {
            list.add(i);
        }

        gs2.push(std::move(list));
    }
    else {
        throw GS2Exception{"Unsupported types for 0x2f (for now)"};
    }
}

// 0x56 - read-num
void readNum(GS2Context &gs2) {
    auto str = makeString(gs2.pop());
    auto numberRegex = std::regex{"-?[0-9]+"};
    std::smatch match;

    if (!std::regex_search(str, match, numberRegex)) {
        throw GS2Exception{"Unable to read a number!"};
    }

    gs2.push(Value::IntType(match.str()));
}

// 0x57 - read-nums
void readNums(GS2Context &gs2) {
    auto str = makeString(gs2.pop());
    auto numberRegex = std::regex{"-?[0-9]+"};

    auto begin = std::sregex_iterator(str.begin(), str.end(), numberRegex);
    auto end = std::sregex_iterator();

    List numbers;
    for (auto it = begin; it != end; ++it) {
        numbers.add(Value::IntType(it->str()));
    }

    gs2.push(std::move(numbers));
}

// 0x52 - show
void show(GS2Context &gs2) {
    auto val = gs2.pop();
    gs2.push(makeList(val.str()));
}

// 0x58 - show-line
void showLine(GS2Context &gs2) {
    auto val = gs2.pop();
    gs2.push(makeList(val.str() + '\n'));
}

// 0x54 - show-lines
void showLines(GS2Context &gs2) {
    auto listVal = gs2.pop();

    if (!listVal.isList()) {
        throw GS2Exception{"Cannot perform show-lines operation on a non-list"};
    }

    std::string str;
    const auto &list = listVal.getList();

    for (size_t i = 0; i < list.size(); i++) {
        str += list[i].str();

        if (i != list.size() - 1) {
            str += '\n';
        }
    }

    gs2.push(makeList(str));
}

// 0x59 - show-space
void showSpace(GS2Context &gs2) {
    auto val = gs2.pop();
    gs2.push(makeList(val.str() + ' '));
}

// 0x55 - show-words
void showWords(GS2Context &gs2) {
    auto listVal = gs2.pop();

    if (!listVal.isList()) {
        throw GS2Exception{"Cannot perform show-words operation on a non-list"};
    }

    std::string str;
    const auto &list = listVal.getList();

    for (size_t i = 0; i < list.size(); i++) {
        str += list[i].str();

        if (i != list.size() - 1) {
            str += ' ';
        }
    }

    gs2.push(makeList(str));
}

// 0x0d - space
void space(GS2Context &gs2) {
    List list;
    list.add(' ');
    gs2.push(std::move(list));
}

// 0x64 - sum / even
void sum(GS2Context &gs2) {
    auto val = gs2.pop();

    if (val.isNumber()) {
        gs2.push(val.getNumber() % 2 == 0 ? 1 : 0);
    }
    else if (val.isList()) {
        Value::IntType sum = 0;

        for (const auto &numVal: val.getList()) {
            if (!numVal.isNumber()) {
                throw GS2Exception{"Cannot sum a list with non-numbers!"};
            }
            sum += numVal.getNumber();
        }

        gs2.push(sum);
    }
    else {
        throw GS2Exception{"Cannot perform sum/even on a block!"};
    }
}

// 0x22 - not / tail
void tail(GS2Context &gs2) {
    auto val = gs2.pop();

    if (val.isNumber()) {
        gs2.push(val.getNumber() ? 0 : 1);
    }
    else if (val.isList()) {
        auto& list = val.getList();
        if (list.empty()) {
            throw GS2Exception{"Cannot get the tail of an empty list!"};
        }
        gs2.push(std::move(list.back()));
    }
    else {
        throw GS2Exception{"Cannot perform not/tail on a block!"};
    }
}

// 0x2b - half / unlines
void unlines(GS2Context &gs2) {
    auto val = gs2.pop();

    if (val.isNumber()) {
        val.getNumber() /= 2;
        gs2.push(std::move(val));
    }
    else if (val.isList()) {
        List joined;
        auto &toJoin = val.getList();

        for (size_t i = 0; i < toJoin.size(); i++) {
            if (i > 0) {
                joined.add('\n');
            }
            for (auto c: toJoin[i].str()) {
                joined.add(c);
            }
        }

        gs2.push(std::move(joined));
    }
    else {
        throw GS2Exception{"Cannot perform half/unlines on a block!"};
    }
}

// 0x84 - uppercase-alphabet
void uppercaseAlphabet(GS2Context &gs2) {
    List list;
    for (int i = 'A'; i <= 'Z'; i++) {
        list.add(i);
    }
    gs2.push(std::move(list));
}

} // namespace gs2
