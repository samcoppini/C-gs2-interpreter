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

} // namespace gs2
