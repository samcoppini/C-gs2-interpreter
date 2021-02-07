#include "gs2context.hpp"

namespace gs2 {

GS2Context::GS2Context(List &stack):
    _stack(stack)
{}

void GS2Context::push(Value value) {
    _stack.add(std::move(value));
}

Value GS2Context::pop() {
    return _stack.pop();
}

} // namespace gs2
