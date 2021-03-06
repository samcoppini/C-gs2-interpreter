#include "utils.hpp"
#include "gs2exception.hpp"
#include "value.hpp"

namespace gs2 {

List makeList(const std::string &str) {
    List list;
    for (auto c: str) {
        list.add(c);
    }
    return list;
}

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

List join(List toJoin, const List &separator) {
    List joined;

    for (size_t i = 0; i < toJoin.size(); i++) {
        if (i > 0) {
            for (const auto &val: separator) {
                joined.add(val);
            }
        }
        if (toJoin[i].isList()) {
            for (auto &item: toJoin[i].getList()) {
                joined.add(std::move(item));
            }
        }
        else {
            joined.add(std::move(toJoin[i]));
        }
    }

    return joined;
}

bool subListEqual(const List &list1, const List &list2, size_t start) {
    if (start + list1.size() > list2.size()) {
        return false;
    }

    for (size_t i = 0; i < list1.size(); i++) {
        if (list1[i] != list2[i + start]) {
            return false;
        }
    }

    return true;
}

List split(List toSplit, const List &sep, bool clean) {
    List result;
    result.add(List{});

    for (size_t i = 0; i < toSplit.size(); i++) {
        if (subListEqual(sep, toSplit, i)) {
            if (!clean || !result.back().getList().empty()) {
                result.add(List{});
            }
            i += sep.size() - 1;
        }
        else {
            result.back().getList().add(std::move(toSplit[i]));
        }
    }

    if (clean && result.back().getList().empty()) {
        result.pop();
    }

    return result;
}

List stepOver(List list, int64_t stepSize) {
    if (stepSize == 0) {
        throw GS2Exception{"Step size cannot be zero!"};
    }

    List newList;

    if (stepSize > 0) {
        for (size_t i = 0; i < list.size(); i += stepSize) {
            newList.add(std::move(list[i]));
        }
    }
    else {
        for (size_t i = list.size() - 1; i < list.size(); i += stepSize) {
            newList.add(std::move(list[i]));
        }
    }

    return newList;
}

} // namespace gs2
