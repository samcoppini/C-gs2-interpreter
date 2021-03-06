#include "gs2context.hpp"
#include "block.hpp"
#include "gs2exception.hpp"

namespace gs2 {

GS2Context::GS2Context(List &stack):
    _stack(stack),
    _counter(1)
{}

void GS2Context::push(Value value) {
    _stack.add(std::move(value));
}

Value GS2Context::pop() {
    return _stack.pop();
}

void GS2Context::dup(size_t indexFromBack) {
    if (_stack.size() <= indexFromBack) {
        throw GS2Exception{"Stack is too small!"};
    }
    push(_stack[_stack.size() - indexFromBack - 1]);
}

int GS2Context::getAndIncCounter() {
    return _counter++;
}

void GS2Context::do_map(const Block &block, List list) {
    auto origSize = _stack.size();

    for (auto &val: list) {
        push(std::move(val));
        block.execute(*this);
    }

    list.clear();
    for (auto i = origSize; i < _stack.size(); i++) {
        list.add(std::move(_stack[i]));
    }

    while (_stack.size() > origSize) {
        _stack.pop();
    }

    _stack.add(std::move(list));
}

} // namespace gs2
