#pragma once

#include "value.hpp"

#include <iosfwd>

namespace gs2 {

class GS2Context {
    private:
        List &_stack;

    public:
        GS2Context(List &stack);

        void push(Value value);

        Value pop();
};

} // namespace gs2
