#include "value.hpp"

namespace gs2 {

Value::Value(int64_t num): _data(num) {
}

Value::Value(List list): _data(std::move(list)) {
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

} // namespace gs2
