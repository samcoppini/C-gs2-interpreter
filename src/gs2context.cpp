#include "gs2context.hpp"

namespace gs2 {

GS2Context::GS2Context(std::ostream &output, List &stack):
    _output(output),
    _stack(stack)
{}

void GS2Context::push(Value value) {
    _stack.add(std::move(value));
}

} // namespace gs2
