#pragma once

#include "value.hpp"

#include <iosfwd>

namespace gs2 {

class GS2Context {
    private:
        List &_stack;

        int _counter;

    public:
        GS2Context(List &stack);

        void push(Value value);

        Value pop();

        int getAndIncCounter();

        void do_map(const Block &block, List val);
};

} // namespace gs2
