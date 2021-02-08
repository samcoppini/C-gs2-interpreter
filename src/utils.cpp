#include "utils.hpp"
#include "gs2exception.hpp"
#include "value.hpp"

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

List split(List toSplit, const List &sep) {
    List result;
    result.add(List{});

    for (size_t i = 0; i < toSplit.size(); i++) {
        if (subListEqual(sep, toSplit, i)) {
            result.add(List{});
            i += sep.size() - 1;
        }
        else {
            result.back().getList().add(std::move(toSplit[i]));
        }
    }

    return result;
}

} // namespace gs2
