#include "list.hpp"
#include "value.hpp"

namespace gs2 {

List::List() {}

List::~List() {}

void List::add(Value value) {
    _values.emplace_back(std::move(value));
}

std::vector<Value>::iterator List::begin() {
    return _values.begin();
}

std::vector<Value>::iterator List::end() {
    return _values.end();
}

Value& List::operator[](size_t index) {
    return _values[index];
}

const Value& List::operator[](size_t index) const {
    return _values[index];
}

size_t List::size() const {
    return _values.size();
}

} // namespace gs2
