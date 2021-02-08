#include "list.hpp"
#include "gs2exception.hpp"
#include "value.hpp"

#include <algorithm>

namespace gs2 {

List::List() {}

List::~List() {}

bool List::operator!=(const List &rhs) const {
    if (_values.size() != rhs.size()) {
        return true;
    }

    for (size_t i = 0; i < rhs.size(); i++) {
        if (_values[i] != rhs[i]) {
            return true;
        }
    }

    return false;
}

void List::add(Value value) {
    _values.emplace_back(std::move(value));
}

void List::concat(const List &list) {
    _values.insert(_values.end(), list._values.begin(), list._values.end());
}

void List::insert(std::vector<Value>::iterator it, Value value) {
    _values.insert(it, std::move(value));
}

Value List::pop() {
    if (_values.empty()) {
        throw GS2Exception{"Cannot pop an empty list!"};
    }

    auto value = std::move(_values.back());
    _values.pop_back();
    return value;
}

void List::reverse() {
    std::reverse(_values.begin(), _values.end());
}

std::vector<Value>::iterator List::begin() {
    return _values.begin();
}

std::vector<Value>::iterator List::end() {
    return _values.end();
}

std::vector<Value>::const_iterator List::begin() const {
    return _values.begin();
}

std::vector<Value>::const_iterator List::end() const {
    return _values.end();
}

Value& List::operator[](size_t index) {
    return _values[index];
}

const Value& List::operator[](size_t index) const {
    return _values[index];
}

Value& List::back() {
    return _values.back();
}

const Value& List::back() const {
    return _values.back();
}

size_t List::size() const {
    return _values.size();
}

bool List::empty() const {
    return _values.empty();
}

} // namespace gs2
