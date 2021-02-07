#include "value.hpp"
#include "gs2exception.hpp"

#include <type_traits>

namespace gs2 {

Value::Value(int64_t num): _data(num) {
}

Value::Value(List list): _data(std::move(list)) {
}

Value::Value(Block block): _data(std::move(block)) {
}

bool Value::isNumber() const {
    return std::holds_alternative<int64_t>(_data);
}

int64_t Value::getNumber() const {
    return std::get<int64_t>(_data);
}

bool Value::isList() const {
    return std::holds_alternative<List>(_data);
}

const List& Value::getList() const {
    return std::get<List>(_data);
}

List& Value::getList() {
    return std::get<List>(_data);
}

bool Value::isBlock() const {
    return std::holds_alternative<Block>(_data);
}

const Block& Value::getBlock() const {
    return std::get<Block>(_data);
}

Block& Value::getBlock() {
    return std::get<Block>(_data);
}

std::string Value::str(bool nested) const {
    return std::visit([nested] (const auto &arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, List>) {
            std::string str;

            for (const auto& val: arg) {
                str += val.str(true);
            }

            return str;
        }
        else if constexpr (std::is_same_v<T, Block>) {
            throw GS2Exception{"Cannot turn a block to string!"};
        }
        else if constexpr (std::is_same_v<T, int64_t>) {
            if (nested) {
                std::string str;
                str += static_cast<char>(arg);
                return str;
            }
            else {
                return std::to_string(arg);
            }
        }
    }, _data);
}

} // namespace gs2
