#include "value.hpp"
#include "gs2exception.hpp"

#include <type_traits>

namespace gs2 {

Value::Value(int64_t num): _data(IntType(num)) {
}

Value::Value(IntType num): _data(std::move(num)) {
}

Value::Value(List list): _data(std::move(list)) {
}

Value::Value(Block block): _data(std::move(block)) {
}

bool Value::operator!=(const Value &rhs) const {
    if (_data.index() != rhs._data.index()) {
        return true;
    }

    return std::visit([&rhs] (const auto &arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, IntType>) {
            return arg != rhs.getNumber();
        }
        else if constexpr (std::is_same_v<T, Block>) {
            return arg != rhs.getBlock();
        }
        else if constexpr (std::is_same_v<T, List>) {
            return arg != rhs.getList();
        }
    }, _data);
}

bool Value::isNumber() const {
    return std::holds_alternative<IntType>(_data);
}

const Value::IntType& Value::getNumber() const {
    return std::get<IntType>(_data);
}

Value::IntType& Value::getNumber() {
    return std::get<IntType>(_data);
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
        else if constexpr (std::is_same_v<T, IntType>) {
            if (nested) {
                std::string str;
                str += static_cast<char>(arg);
                return str;
            }
            else {
                return arg.str();
            }
        }
    }, _data);
}

} // namespace gs2
