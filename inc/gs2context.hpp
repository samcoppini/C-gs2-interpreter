#pragma once

#include "value.hpp"

#include <iosfwd>

namespace gs2 {

class GS2Context {
    private:
        std::ostream &_output;

        List &_stack;

    public:
        GS2Context(std::ostream &output, List &stack);

        void push(Value value);

        Value pop();
};

} // namespace gs2
