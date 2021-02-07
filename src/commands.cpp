#include "commands.hpp"
#include "gs2context.hpp"

#include <cassert>

namespace gs2 {

// 0x20 command
// negate a number
// reverse a list
// evaluate a block
void negate(GS2Context &gs2) {
    auto value = gs2.pop();

    if (value.isNumber()) {
        gs2.push(-value.getNumber());
    }
    else if (value.isList()) {
        value.getList().reverse();
        gs2.push(std::move(value));
    }
    else {
        assert(value.isBlock());
        value.getBlock().execute(gs2);
    }
}

} // namespace gs2
