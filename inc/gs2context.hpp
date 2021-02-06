#pragma once

#include "value.hpp"

#include <iosfwd>

namespace gs2 {

class GS2Context {
    private:
        std::istream &_input;

        std::ostream &_output;

        List &_stack;

    public:
        GS2Context(std::istream &input, std::ostream &output, List &list);

        void push(Value value);
};

} // namespace gs2
